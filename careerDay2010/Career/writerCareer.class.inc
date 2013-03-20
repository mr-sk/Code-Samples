<?php
/**
 * @author  Ben Sgro
 */

include_once('career.class.inc');

/**
 * @package writerCareer
 */
class writerCareer extends career
{
    function writerCareer()
    {
        parent::career(get_class($this));
    }
    
    function updateStatusForLoggedInUser()
    {
        parent::updateStatusForLoggedInUser();
    }
}