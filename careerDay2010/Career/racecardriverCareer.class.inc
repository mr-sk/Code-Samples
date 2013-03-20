<?php
/**
 * @author  Ben Sgro
 */

include_once('career.class.inc');

/**
 * @package racecardriverCareer
 */
class racecardriverCareer extends career
{
    function racecardriverCareer()
    {
        parent::career(get_class($this));
    }
    
    function updateStatusForLoggedInUser()
    {
        parent::updateStatusForLoggedInUser();
    }
}