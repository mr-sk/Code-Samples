<?php
/**
 * @author  Ben Sgro
 */

include_once('career.class.inc');

/**
 * @package policeCareer
 */
class policeCareer extends career
{
    function policeCareer()
    {
        parent::career(get_class($this));
    }
    
    function updateStatusForLoggedInUser()
    {
        parent::updateStatusForLoggedInUser();
    }
}