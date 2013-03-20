<?php
/**
 * @author  Ben Sgro
 */

include_once('career.class.inc');

/**
 * @package parkrangerCareer
 */
class parkrangerCareer extends career
{
    function parkrangerCareer()
    {
        parent::career(get_class($this));
    }
    
    function updateStatusForLoggedInUser()
    {
        parent::updateStatusForLoggedInUser();
    }
}