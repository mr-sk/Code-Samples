<?php
/**
 * @author Ben Sgro
 */
//include_once('StateMachine/stateMachine.php');

class career
{
    /**
     * The following values are set by the .ini file:
     */
    var $_id                   = null;
    var $_image                = null;
    var $_title                = null;
    var $_description          = null;

    var $_taskOne              = null;
    var $_taskTwo              = null;
    var $_taskThree            = null;

    var $_taskOneMessage       = null;
    var $_taskTwoMessage       = null;
    var $_taskThreeMessage     = null;
    
    var $_taskOneDescription   = null;
    var $_taskTwoDescription   = null;
    var $_taskThreeDescription = null;

    var $_taskOneAwardItemId   = null;
    var $_taskTwoAwardItemId   = null;
    var $_taskThreeAwardItemId = null;
        
    var $_taskOneAwardItem     = null;
    var $_taskTwoAwardItem     = null;
    var $_taskThreeAwardItem   = null;

    var $_taskOneAwardCheck    = null;
    var $_taskTwoAwardCheck    = null;
    var $_taskThreeAwardCheck  = null;
    
    var $_taskOneAwardTitle    = null;
    var $_taskTwoAwardTitle    = null;
    var $_taskThreeAwardTitle  = null;
    
    var $_taskOneGameId        = null;
    var $_taskTwoGameId        = null;
    var $_taskThreeGameId      = null;
    
    var $_taskOneTargetType    = null;
    var $_taskTwoTargetType    = null;
    var $_taskThreeTargetType  = null;
    
    var $_taskOneTargetURL     = null;
    var $_taskTwoTargetURL     = null;
    var $_taskThreeTargetURL   = null;
        
    var $_taskOneGameScore     = null;
    var $_taskTwoGameScore     = null;
    var $_taskThreeGameScore   = null;

    var $_taskOneGameWinSwf    = null;
    var $_taskTwoGameWinSwf    = null;
    var $_taskThreeGameWinSwf  = null;

    var $_taskOneGameLoseSwf   = null;
    var $_taskTwoGameLoseSwf   = null;
    var $_taskThreeGameLoseSwf = null;

    var $_taskCompleteMessage  = null;  
    // End .ini
    
    /**
     * The following values are NOT set by the .ini file:
     */
    var $_stateMachine         = null;
    
    var $_taskOneComplete      = 0;
    var $_taskTwoComplete      = 0;
    var $_taskThreeComplete    = 0;
    
    var $_taskStartTime        = 0;
    
    var $_stayInState          = FALSE;
    // End NOT .ini
    
    /**
     * Called when we create an instance of any of the subclasses, this method
     * will attempt to load the .ini for the specific career and populate the
     * instance with all the data from the .ini file.
     *
     * It will also provide the '_stateMachine' member variable with an instance
     * of the statemachine.
     *
     * If the .ini file cannot be located for the career, it will die.
     */
    function career()
    {
        // Based on the class, attempt to load the accompaning ini file
        $iniFile = sprintf(ROOT . CD_PATH . CAREER_PATH . 'data/%s.ini', str_replace('career', 'Career', get_class($this)));
        if (file_exists($iniFile))
        {
            _trace(sprintf('Loading: %s', $iniFile));            
            $set = parse_ini_file($iniFile);
            foreach($set as $key => $value)
            {
                $this->$key = $value;
            }
        }
        else
        {
            _trace(sprintf("Error locating file: %s", $iniFile));
            die();
        }

        include_once(ROOT . CD_PATH . 'StateMachine/stateMachine.class.inc');
        $this->_stateMachine = new stateMachine();
    }
    
   /**
     * If the currently logged in user hasn't started the career, this will return
     * false. Its used for the hooking system to make sure we don't do any more processing
     * then is necessary.
     */
    function isActiveForUser()
    {
        $sql = sprintf("SELECT job_id FROM %s WHERE job_id = %s and username = '%s'",
                       CAREER_TABLE,
                       $this->_id,
                       USERNAME);
        if (db_num_rows(db_query($sql)) > 0)
        {
            _trace('Career active for user');
            return TRUE;
        }

        _trace('Career not active for user');        
        return FALSE;
    }
    
    /**
     * Get the int value of the current state and look up that in the database. If its set,
     * the user is allowed to complete the task, if not, they cannot.
     *
     * @return  bool    TRUE if task is active for user, FALSE otherwise.
     */
    function isTaskActiveForUser()
    {
        $taskNumber = $this->_stateMachine->getStateAsInteger()+1;          
        $taskActive = sprintf('task_%s_active', $taskNumber);
        $sql = sprintf("SELECT %s FROM %s WHERE job_id = %s AND username = '%s' AND %s = 1",
                       $taskActive,
                       CAREER_TABLE,
                       $this->_id,
                       USERNAME,
                       $taskActive);
        _trace($sql);
        $set = db_fetch_assoc(db_query($sql));
        if (1 == @ $set[$taskActive])
        {
            _trace('Task active for user');
            return TRUE;
        }
        
        _trace('Task not active for user');
        return FALSE;
    }
    
    /**
     * Check if the currently logged in user has already started a job today.
     *
     * Since we removed the one task per day limit, this function wasn't returning
     * data correctly. 
     *
     * @return  bool    TRUE if the user already started a job today
     *                  FALSE if they did not
     */
    function hasAlreadyStartedAJobToday()
    {
        $sql = sprintf("SELECT task_start FROM %s WHERE username = '%s'",
                     //. " AND (task_1 = 1 OR task_1 = 0) AND (task_2 = 0 and task_3 = 0)",
                     CAREER_TABLE,
                     USERNAME);
        $res = db_query($sql);
        _trace($sql);
        while($entry = db_fetch_assoc($res))
        {
            $taskStart = @ $entry['task_start'];
            if (date('z-Y', $taskStart) == date('z-Y')) //0->365-YYYY
            {
                _trace('User already started a job today');
                return TRUE;
            }
        }
        
        _trace('User has not started a job today');
        return FALSE;
    }    
    
    /**
     * Note, this is per career. We enforce that only one task per career
     * can be completed per day - and we define a day as 12:00AM -> 12:00PM,
     * So wed 11:59PM VS thurs 12:01AM are two different day.
     * 
     * Get the numerical day of the year and the year and compare them against
     * the time the task was started in the database. If they match, the user
     * has already started a task today and we cannot let them start another one.
     *
     * @return  boolean     TRUE if the user has completed a task for this career
     *                      today - FALSE if they have not.
     */
    function hasAlreadyCompletedATaskToday()
    {
        $sql = sprintf("SELECT task_start FROM %s WHERE job_id = %s AND username = '%s'"
                     . " AND (task_1 = 1 OR task_2 = 1 OR task_3 = 1)",
                       CAREER_TABLE,
                       $this->_id,
                       USERNAME);
        $set = db_fetch_assoc(db_query($sql));

        $taskStart = @ $set['task_start'];        
        if (date('z-Y', $taskStart) == date('z-Y')) //0->365-YYYY
        {
            _trace('User already completed a task for this career today');
            return TRUE;
        }
        
        return FALSE;
    }
    
    
    /**
     * Note, this is global.
     *
     * If the user has completed task 3 and the date is today, that means they've completed
     * the job (today) - and we only allow ONE job to be completed per day.
     *
     * @return  boolean     TRUE if the user has completed a job today for any career
     *                      FALSE if they have not
     */
    function hasAlreadyCompletedAJobToday()
    {
        $sql = sprintf("SELECT task_start FROM %s WHERE username = '%s' AND task_3 = 1",
                       CAREER_TABLE,
                       USERNAME);
        $set = db_fetch_assoc(db_query($sql));
        
        $taskStart = @ $set['task_start'];
        if (date('z-Y', $taskStart) == date('z-Y'))
        {
            _trace('User has already completed a job today');
            return TRUE;
        }
        
        return FALSE;
    }
    
    /**
     * This method will find if the user has begun working on the provided career
     * and attempt to correctly setup the state machine.
     */
    function updateStatusForLoggedInUser()
    {
        $sql = sprintf("SELECT * FROM %s WHERE username = '%s' AND JOB_ID = %s",
                       CAREER_TABLE,
                       USERNAME,
                       $this->_id);
        _trace($sql);
        $res = db_query($sql);
        $res = db_fetch_assoc($res);
        if (empty($res))
        {
            // User has not started on this job, set the default state, which is
            // __CAREER__taskOne
            $this->_stateMachine->changeState($this->initDefaultStateClass());
        }
        else
        {
            // Get the job_id and sum all the tasks, then map the current task
            // to a career and state and set that up in this instance of the
            // state machine.
            $jobId   = $res['job_id'];
            $jobTask = $res['task_1'] + $res['task_2'] + $res['task_3']; 
            $this->_taskOneComplete   = $res['task_1'];
            $this->_taskTwoComplete   = $res['task_2'];
            $this->_taskThreeComplete = $res['task_3'];
        
            // Get the time when we started the most current task
            $this->_taskStartTime     = $res['task_start'];
            list($state, $job) = career::mapTaskToStateClass($jobId, $jobTask);
            $state = career::initStateClass($job, $state);
            $this->_stateMachine->changeState($state);
        }
    }

    /**
     * Similiar to the other hook methods we've created. This one does no validation and just
     * advances to the next state and provides swg arguments so the swf layer can display
     * the check.
     */
    function hookShopItem()
    {
        _trace('hookShopItem()');
       
        global $gFlashArgs;
        
        // Move to the next state
        $currentStateId = $this->_stateMachine->getStateAsInteger();                
        list($state, $job) = career::mapTaskToStateClass($this->_id, $currentStateId + 1);        
        include_once(sprintf('%s%sStateMachine/States/%s%s.class.inc', ROOT, CD_PATH, $job, $state));
        $newState = sprintf('%s%s', $job, $state);
        $c = new $newState;
        $this->_stateMachine->changeState($c, $this);            
        
        // Generate the response arguments/XML
        $gameWinSwf = sprintf('_%sGameWinSwf', TransformXML::lcfirst(career::mapIntegerToTaskString($currentStateId)));
        $swfArgs    = sprintf('?xml=%s', urlencode(TransformXML::completeTaskXML($this)));
        $gFlashArgs = sprintf('&extensionMovieClip=%s',
                             urlencode(IMAGE_HOST . $this->$gameWinSwf . $swfArgs));
        
        _trace('exit hookShopItem()');
    }    
    
    /** 
     * The studio hook is called when the tshirt factory submits a purchase command. The extension
     * is run and if the item created matches our requirements, this will be called if the user
     * is in the correct state.
     *
     * Its similiar to all the other hooks in that it sets the global $hookVars w/the swf and arguments.
     */
    function hookStudio()
    {
        _trace('hookStudio()');

        global $hookVars;  // The variable we use to append swf arguments
        global $shirtType; // The shirt type, set in the @see studioProcessExtension.inc
        
        $currentStateId = $this->_stateMachine->getStateAsInteger();        
        $shirtId = sprintf('_%sShirtId', TransformXML::lcfirst(career::mapIntegerToTaskString($currentStateId)));
        if ($this->$shirtId == $shirtType)
        {
            list($state, $job) = career::mapTaskToStateClass($this->_id, $currentStateId + 1);
    
            include_once(sprintf('%s%sStateMachine/States/%s%s.class.inc', ROOT, CD_PATH, $job, $state));
            $newState = sprintf('%s%s', $job, $state);
            $c = new $newState;
            $this->_stateMachine->changeState($c, $this);            
    
            // Generate the response arguments/XML
            $gameWinSwf = sprintf('_%sGameWinSwf', TransformXML::lcfirst(career::mapIntegerToTaskString($currentStateId)));
            $swfArgs    = sprintf('?xml=%s', urlencode(TransformXML::completeTaskXML($this)));
            $hookVars   = sprintf('&extensionMovieClip=%s',
                                 urlencode(IMAGE_HOST . $this->$gameWinSwf . $swfArgs));
        }
        else
        {
            _trace(sprintf('You created shirt (%s) but shirt (%s) was required', $shirtType, $this->$shirtId));

            $gameWinSwf = sprintf('_%sGameWinSwf', TransformXML::lcfirst(career::mapIntegerToTaskString($currentStateId)));
            $swfArgs    = sprintf('?xml=%s', urlencode('<root/>'));
            $hookVars   = sprintf('&extensionMovieClip=%s',
                                urlencode(IMAGE_HOST . $this->$gameWinSwf . $swfArgs));             
        }

        _trace('exit hookStudio()');
    }
    
    /**
     * There's two high level ways execution can get to this point.
     * 1) Called via the API
     * 2) Called via the extension
     *
     * If we've been called from the API, we assume validation passed, the book id is correct,
     * this user hasn't completed any tasks in this career today and that they are active in
     * this career. So, we can just move them to the next state class.
     *
     * If we've been called from the extension (this is currently) we just want to return data
     * for the book swf loader - but we don't move to the next state yet.
     */
    function hookBook()
    {
        _trace('hookBook()');
        
        if ($this->_stayInState)
        {
            _trace('stayInState');
            // In this case, we came from the bookExtension.inc
            // and we just want to extract and return the swf data
            // for the book - we don't want to move to the next state.
            global $gBookId;
            global $gReturnValue;
            
            // Fine the position of ...
            $pos = strpos($gReturnValue, '&serverTimeOffset=');
            
            // Get everthing before the stirng
            $beforePos = substr($gReturnValue, 0, $pos);
            
            // Get everything after the string
            $afterPos = substr($gReturnValue, $pos, strlen($gReturnValue));
            
            // Generate our extension arguments for the swf.
            $currentStateId = $this->_stateMachine->getStateAsInteger();
            $gameWinSwf     = sprintf('_%sGameWinSwf', TransformXML::lcfirst(career::mapIntegerToTaskString($currentStateId)));
            $args           = sprintf('&extensionMovieClip=%s',
                                            urlencode(IMAGE_HOST . $this->$gameWinSwf));            
            // Re-assemble the string.
            $gReturnValue = $beforePos . $args . $afterPos;            
        }
        else
        {
            _trace('going to next state');
            // In this case we came from the API, and we can go to the next state.
            list($state, $job) = career::mapTaskToStateClass($this->_id,
                               (($this->_stateMachine->getStateAsInteger())+1)); // we want the NEXT state
        
            include_once(sprintf('%s%sStateMachine/States/%s%s.class.inc', ROOT, CD_PATH, $job, $state));
            $newState = sprintf('%s%s', $job, $state);
            $c = new $newState;
            $this->_stateMachine->changeState($c, $this);
        }        
    }
    
    /**
     * This, luckily is currently a pretty generic method that allows us to hook the score
     * system. It access two global variables in the submit_score code:
     * 
     *  $hookVars   If we choose to, we can alter the request to the game swf.
     *  $FG_SCORE  The score
     *
     * Depending on the state we are in, we load different values from the .ini config
     * file for the career and perform evaluations against the score, and then we append
     * a URI param to the response for a swf to be shown.
     */
    function hookGameAndEvalScore()
    {
        _trace('hookGameAndEvalScore()');
        
        global $hookVars;
        global $FG_SCORE;

        $currentStateId = $this->_stateMachine->getStateAsInteger();
        $gameScore = sprintf('_%sGameScore', TransformXML::lcfirst(career::mapIntegerToTaskString($currentStateId)));

        _trace('fg_score = ' . $FG_SCORE);
        _trace('gameScore = ' . $this->$gameScore);

        if ($FG_SCORE >= $this->$gameScore)
        {
            // Advance to the next state            
            // Increment the current state to determine where we go next.
            list($state, $job) = career::mapTaskToStateClass($this->_id, $currentStateId + 1);

            include_once(sprintf('%s%sStateMachine/States/%s%s.class.inc', ROOT, CD_PATH, $job, $state));
            $newState = sprintf('%s%s', $job, $state);
            $c = new $newState;
            $this->_stateMachine->changeState($c, $this);            

            // Generate the response arguments/XML
            $gameWinSwf = sprintf('_%sGameWinSwf', TransformXML::lcfirst(career::mapIntegerToTaskString($currentStateId)));
            $swfArgs    = sprintf('?xml=%s', urlencode(TransformXML::completeTaskXML($this)));
            $hookVars = sprintf('&extensionMovieClip=%s',
                                urlencode(IMAGE_HOST . $this->$gameWinSwf . $swfArgs));
        }
        else
        {
            _trace('You did NOT beat the score!');

            $gameWinSwf = sprintf('_%sGameWinSwf', TransformXML::lcfirst(career::mapIntegerToTaskString($currentStateId)));
            $swfArgs    = sprintf('?xml=%s', urlencode('<root/>'));
            $hookVars   = sprintf('&extensionMovieClip=%s',
                                urlencode(IMAGE_HOST . $this->$gameWinSwf . $swfArgs));            
        }                        
    }
    
    /**
     * Called when the user starts a new task on the road to completion of a career.
     * Mark the first task as active.
     */
    function startTask()
    {
        $sql = sprintf("INSERT INTO %s (job_id, username, task_start, task_1_active)"
                    .  " VALUES(%s, '%s', %s, %s)",
                    CAREER_TABLE,
                    $this->_id,
                    USERNAME,
                    NOW,
                    1);
        _trace($sql);
        $res = db_query($sql);        
    }
    
    /**
     * For the current career, update the  current task we are one set it to complete (1)
     * and also update the start_task time to be the current time.
     *
     * This function should be called from the exitState() of a state.
     */
    function updateTask()
    {
        $taskNumber = $this->_stateMachine->getStateAsInteger();
        // These start at 0 for the first state, 1 for the second, etc,
        // so we need to increment to get the correct task in the database
        $taskNumber++;

        $sql = sprintf("UPDATE %s SET task_%s = 1, task_start = %s WHERE username = '%s' AND job_id = %s",
                       CAREER_TABLE,
                       $taskNumber,
                       NOW,
                       USERNAME,
                       $this->_id);
        _trace($sql);
        $res = db_query($sql);        
    }

    /**
     * For the current task, give the user the item defined in the __career__.ini file.
     * Items are defined in:
     *  __task__AwardItemId = ...
     */
    function giveItem()
    {
        $taskCount = TransformXML::lcfirst(
                                           career::mapIntegerToTaskString(
                                                        $this->_stateMachine->getStateAsInteger()));

        $itemStr = sprintf('_%sAwardItemId', $taskCount);
        _trace(sprintf('Giving user %s item %s', USERNAME, $this->$itemStr));
        give_item_new(USERNAME, $this->$itemStr);
    }

    /**
     * For the current task, give the user the amount of Millsbucks defined in the __career_.ini file
     * Millsbucks are defined in:
     *  __task__AwardCheck = ...
     */
    function giveCheck()
    {
        $taskCount = TransformXML::lcfirst(
                                           career::mapIntegerToTaskString(
                                                        $this->_stateMachine->getStateAsInteger()));

        $itemStr = sprintf('_%sAwardCheck', $taskCount);
        _trace(sprintf('Giving user %s MBs %s', USERNAME, $this->$itemStr));
        give_millsbucks_new(USERNAME, $this->$itemStr);
    }
    
    /**
     * Generate a list of task_1's that have been started. Used to track how many jobs
     * we can start per day.
     *
     * @return  array   Empty or a list of all the careers with the first task set and the
     *                  time of setting.
     */
    function getTaskOneStatusAndTime()
    {
        $sql = sprintf("SELECT task_1, task_start, job_id FROM %s WHERE username = '%s'",
                       CAREER_TABLE,
                       USERNAME);
        $tmpSet = array();
        $res    = db_query($sql);
        while($item = db_fetch_assoc($res))
        {
            $tmpSet[] = $item;
        }
        return $tmpSet;
    }
    
    /**
     * Used to map a job id and task id to the actual state class that we will instantiante
     * and load into the state machine.
     *
     * @param   int     $jobId  A numerical representation of a job/career.
     * @param   string  $task   The state/task the user is currently on.
     *
     * @return  mixed           Array of the correct career and state/task properly formatted
     *                          or exit on failure - shouldn't ever happen in production, heh.
     */
    function mapTaskToStateClass($jobId, $task)
    {
        $stateTask = career::mapIntegerToTaskString($task);        
        $stateJob  = career::mapCareerIdToClass($jobId);
        
        if ('' == $stateJob || '' == $stateTask)
        {
            _trace("Really bad, exiting()");
            exit();
        }
        
        return array($stateTask, $stateJob);
    }
    
    /**
     * Used to map a state (int) to a class name (string). We can pre-pend the
     * career to get the desired state class filename.
     *
     * @param   int $id An integer that represents the current state.
     *
     * @return  string  A string representation of the state, w/out the career.
     */
    function mapIntegerToTaskString($id)
    {
        switch($id)
        {
            case 0: return 'TaskOne';
            case 1: return 'TaskTwo';   
            case 2: return 'TaskThree'; 
            case 3: return 'Complete';  
            default:
                _trace("Unknown id ($id)");
        }
    }
    
    /**
     * Add a new career entry here:
     * ----------------------------
     * 
     * @param   int $id A career id
     *
     * @return  string  For the provided $id return a string to use when creating the class.
     */
    function mapCareerIdToClass($id)
    {
        switch($id)
        {
            case 1: return 'policeCareer';   
            case 2: return 'baseballCareer'; 
            case 3: return 'writerCareer';   
            case 4: return 'parkrangerCareer';
            case 5: return 'astronautCareer';
            case 6: return 'designerCareer';
			case 7: return 'surferCareer';
			case 8: return 'footballplayerCareer';
			case 9: return 'racecardriverCareer';
            default:
                _trace("Unknown careerId($id)");
        }
    }
    
    /**
     * NOTE:
     * If you update this, please update @see defines.inc -> VALID_BOOK_IDS
     *      5912, // Sports Legends, Then and Now: Lou Gehrig
     *      9138, // Writer's Life: Zora Nearle Hurston
     *      11276 // A history of earth day
     *       7150 // X Prize
     *
     * @param   int $bookId A bookId
     *
     * @return  mixed   The career id that is required to read the book, or
     *                  null if an invalid bookId is provided.
     */
    function mapBookIdToCareer($bookId)
    {
        switch($bookId)
        {
            case BOOK_LOUGEHRIG:
                return ID_BASEBALL;
                
            case BOOK_ZORA:
                return ID_WRITER;
            
            case BOOK_EARTHDAY:
            case BOOK_EARTHDAY2:
                return ID_RANGER;
            
            case BOOK_XPRIZE:
                return ID_ASTRONAUT;
            
            default:
                return null;            
        }
    }
    
    /**
     * Used to create an instance of a particular jobs's task state
     * for the state machine.
     *
     * @param   string  $job    A career in a particular format (policeCareer, etc).
     * @pararm  string  $state  A state (aka task) for the career.
     *
     * @return  object  An instance of a state class.
     */
    function initStateClass($job, $state)
    {
        $stateFile = sprintf('%s%s.class.inc', $job, $state);
        include_once(sprintf(ROOT . CD_PATH . 'StateMachine/States/%s', $stateFile));
        $stateClass = sprintf('%s%s', $job, $state);
        $sc = new $stateClass;
        return $sc;
    }
    
    /**
     * Used to set the state machine to its default value.
     *
     * @return  object  An instance of a state class.
     */
    function initDefaultStateClass()
    {
        $stateFile = sprintf('%sTaskOne.class.inc', str_replace('career', 'Career', get_class($this)));
        include_once(sprintf(ROOT . CD_PATH . 'StateMachine/States/%s', $stateFile));
        $stateClass = sprintf('%sTaskOne', get_class($this));
        $sc = new $stateClass; 
        return $sc;
    }
}