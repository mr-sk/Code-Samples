<?php
/**
 * @author  Ben Sgro
 */

include_once('career.class.inc');

/**
 * @package footballplayerCareer
 */
class footballplayerCareer extends career
{
    function footballplayerCareer()
    {
        parent::career(get_class($this));
    }
    
    function updateStatusForLoggedInUser()
    {
        parent::updateStatusForLoggedInUser();
    }
}