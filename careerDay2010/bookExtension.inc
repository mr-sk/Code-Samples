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

function handleBook($jobId)
{
    _trace('handleGame for jobId:' . $jobId);
    $careerString = career::mapCareerIdToClass($jobId);
    include_once(ROOT . CD_PATH . 'StateMachine/stateMachine.class.inc');
    include_once(ROOT . CD_PATH . CAREER_PATH . $careerString . '.class.inc');

    $career = new $careerString;
    if ($career->isActiveForUser())
    {
        $career->_stayInState = TRUE; // Lets the state know that we don't want it to move
                                      // to the next state, since we came from the bookExtension
                                      // not the API.
        $career->updateStatusForLoggedInUser();
        if (!$career->isTaskActiveForUser()) return FALSE;
        
        $career->_stateMachine->update($career);
    }    
}

// Hook System Start
_trace('book-hook system starting');

global $gBookId;
global $gReturnValue;

if ('' == $gBookId) return;

switch ($gBookId)
{
    case BOOK_EARTHDAY:
    case BOOK_EARTHDAY2:
        handleBook(ID_RANGER);
    break;

    case BOOK_LOUGEHRIG:
        handleBook(ID_BASEBALL);
    break;

    case BOOK_ZORA:
        handleBook(ID_WRITER);
    break;

    case BOOK_XPRIZE:
        handleBook(ID_ASTRONAUT);
    break;
}

// Hook System End
_trace('book-hook system ending');