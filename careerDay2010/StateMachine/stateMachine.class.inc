<?php
/**
 * @author Ben Sgro
 */
include_once('state.class.inc');

/**
 * @package stateMachine
 */
class stateMachine
{
    var $_currentState  = null;
    var $_previousState = null;
    var $_globalState   = null;

    /**
     * Classes implementing the state machine will only call 'update' on the state
     * machine instance in the career. The state machine will take the responsiblity
     * of exiting, entering and executing classes.
     *
     * The actual state implementations will take care of the logic to determine if
     * we can change states.
     *
     * $career->_stateMachine->update($career);
     *
     * @param   object  $career An instance of the career.
     */
    function update($career)
    {
        if (is_object($this->_currentState))
        {
            $this->_currentState->executeState($career);
        }
        else
        {
            _trace('_currentState is not set, cannot call executeState()');
        }
    }
    
    /**
     * Allows us to change states. We track the previous state
     * by storing it in the _previousState property.
     *
     * Also, we only call exitState() if we are currently IN a state,
     * so if we are just starting off, we won't currently be in a
     * state and exitState() will not get called.
     *
     * @param   object  $state  The desired next state.
     * @param   object  $career OPTIONAL: An intance of the career this is
     *                          acting on. Is not needed for the first state
     *                          but is needed for changing states after the first.
     *
     *                          Sort of a design flaw, this class should just have a pointer
     *                          to the instance of the career calling it.
     */
    function changeState($state, $career = '')
    {
        _trace('ChangeState called ...');
        $this->_previousState = $this->_currentState;
        if (is_object($this->_currentState))
        {
            $this->_currentState->exitState($career);
        }
        
        $this->_currentState = $state;
        $this->_currentState->enterState($career);        
    }

    /**
     * Allows us to see if we are in a particular state.
     *
     * @param   string  $state  Any state.
     *
     * @return  bool            TRUE if we are in the $state, false otherwise.
     */
    function isInState($state)
    {
        return ($state == $this->_currentState);   
    }
    
    /**
     * Allows us to get an integer representation of the current state class we are in.
     * Used to help find state classes, object properties, etc.
     *
     * @return  int     -1 on error, >= 0  && <= 3 on success
     */
    function getStateAsInteger()
    {
        $className = get_class($this->_currentState);
        if ($className == null) { _trace('State never initiated properly'); return -1; } // Probably shouldn't ever get here
        if (strstr($className, 'taskone'))   return 0;
        if (strstr($className, 'tasktwo'))   return 1;
        if (strstr($className, 'taskthree')) return 2;
        if (strstr($className, 'complete'))  return 3;
    }    
}