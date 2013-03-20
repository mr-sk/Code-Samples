<?php
/**
 * @author Ben Sgro
 */

/**
 * Abstract base class that all states extend.
 */
class state
{
    /**
     * abstract
     */    
    function enterState($id)
    {
       assert(!"Extend and subclass me!");        
    }

    /**
     * abstract
     */    
    function executeState($id)
    {
       assert(!"Extend and subclass me!");         
    }
    
    /**
     * abstract
     */
    function exitState($id)
    {
        assert(!"Extend and subclass me!");
    }
}