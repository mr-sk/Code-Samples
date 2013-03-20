<?php
/**
 * @author  Ben Sgro
 */

include_once('career.class.inc');

/**
 * @package surferCareer
 */
class surferCareer extends career
{
    function surferCareer()
    {
        parent::career(get_class($this));
    }
    
    function updateStatusForLoggedInUser()
    {
        parent::updateStatusForLoggedInUser();
    }
}