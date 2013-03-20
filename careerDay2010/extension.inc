<?php
/**
 * @author Ben Sgro
 */

define('CD_PATH',      '/campaigns/careerDay2010/');
define('CAREER_PATH',  'Career/');
define('REP_PATH',     ROOT . CD_PATH . 'representations/');

include_once(ROOT . '/global.inc');
include_once(ROOT . '/home/home_func.inc');
include_once(ROOT . CD_PATH . 'careerDayXML.class.inc');
include_once(ROOT . CD_PATH . 'careerDaySPI.class.inc');
include_once(ROOT . CD_PATH . 'TransformXML.class.inc');
include_once(ROOT . CD_PATH . 'defines.inc');
include_once(ROOT . CD_PATH . CAREER_PATH . 'career.class.inc');

/**
 * Creates an instance of the career object and checks if the user
 * is currently active in that career. If they are not we don't do
 * any additional processing. If they are we load their task specific
 * data from the database and tell the state machine to update - this
 * will run the task state code for the current task they are active
 * in and if it needs to hook/process the score, it will be done.
 *
 * If the current state/task doesn't care about the score, nothing will happen.
 *
 * @param   int $jobId  An integer representation of the career.
 * @param   int $gameId The current game id (only really used for COP)
 *
 * @return  void
 */
function handleGame($jobId, $gameId = '')
{
    _trace('handleGame for jobId:' . $jobId);
    $careerString = career::mapCareerIdToClass($jobId);
    include_once(ROOT . CD_PATH . CAREER_PATH . $careerString . '.class.inc');
    include_once(ROOT . CD_PATH . 'StateMachine/stateMachine.class.inc');

    $career = new $careerString;
    if ($career->isActiveForUser())
    {
        $career->updateStatusForLoggedInUser();
        if (!$career->isTaskActiveForUser()) return FALSE;
        
        // If the player is the cop, we have to make sure they are doing the
        // games in the correct order. The cop is currently the only career
        // we have to check this for.
        if (ID_POLICE == $jobId)
        {
            // NOTE:
            // If you change the values here, you probably need to update
            // policeCareer.ini
            if (GAME_ROPE_RACE   == $gameId) $gId = 100; // ini file mappings
            if (GAME_MOST_WANTED == $gameId) $gId = 16;  // ini file mappings

            $iniGameId = sprintf('_%sGameId', TransformXML::lcfirst(career::mapIntegerToTaskString($career->_stateMachine->getStateAsInteger())));
            if ($career->$iniGameId == $gId)
            {
                $career->_stateMachine->update($career);
            }
            else
            {
                _trace(sprintf('Incorrect game %s %s %s', $gameId, $gId, $career->$iniGameId));
                return FALSE;
            }
        }
        
        $career->_stateMachine->update($career);        
    }
}

// Hook System Start
_trace('hook system starting');

$gameId = isset($_REQUEST['gf']) ? $_REQUEST['gf'] : null;
if ($gameId == null)
{
    // Search for the other URI params that might be the game id:
    // Racecar game
    // Football game
    // Surfer game
    $gameId = isset($_REQUEST['gmd_g']) ? $_REQUEST['gmd_g'] : null;
}
if (!$gameId) return;

switch($gameId)
{
    case GAME_MOST_WANTED:
    case GAME_ROPE_RACE:
        handleGame(ID_POLICE, $gameId);
    break;

    case GAME_HOME_RUN:
        handleGame(ID_BASEBALL);
    break;

    case GAME_WORD_MATCH:
        handleGame(ID_WRITER);
    break;

    case GAME_PARK_CLEANUP:
        handleGame(ID_RANGER);
    break;

    case GAME_COUNTDOWN:
        handleGame(ID_ASTRONAUT);
    break;

    case GAME_RACE_CAR:
        handleGame(ID_CARDRIVER);
    break;

    case GAME_FOOTBALL:
        handleGame(ID_FOOTBALL);
    break;

    case GAME_KAI_SURF:
        handleGame(ID_SURFER);
    break;
}

// Hook System End
_trace('hook system ending');