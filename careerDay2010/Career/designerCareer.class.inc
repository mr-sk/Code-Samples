<?php
/**
 * @author  Ben Sgro
 */

include_once('career.class.inc');

/**
 * @package designerCareer
 */
class designerCareer extends career
{
    function designerCareer()
    {
        parent::career(get_class($this));
    }
    
    function updateStatusForLoggedInUser()
    {
        parent::updateStatusForLoggedInUser();
    }
}