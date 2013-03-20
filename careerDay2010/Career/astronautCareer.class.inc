<?php
/**
 * @author  Ben Sgro
 */

include_once('career.class.inc');

/**
 * @package astronautCareer
 */
class astronautCareer extends career
{
    function astronautCareer()
    {
        parent::career(get_class($this));
    }
    
    function updateStatusForLoggedInUser()
    {
        parent::updateStatusForLoggedInUser();
    }
}