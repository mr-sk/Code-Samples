<?php
/**
 * @author  Ben Sgro
 */

include_once('career.class.inc');

/**
 * @package baseballCareer
 */
class baseballCareer extends career
{
    function baseballCareer()
    {
        parent::career(get_class($this));
    }
    
    function updateStatusForLoggedInUser()
    {
        parent::updateStatusForLoggedInUser();
    }
}