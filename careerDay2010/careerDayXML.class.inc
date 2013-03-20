<?php
/**
 * @author  Ben Sgro
 */

include_once('TransformXML.class.inc');

/**
 * @package careerDayXML
 */
class careerDayXML
{
    /**
     *
     */
    function activateTask()
    {
        $res = careerDaySPI::activateTask();
        if ($res < 0)
        {
            return TransformXML::codeResultToXML($res);
        }        
        
        // Send back an instance of the job
        $career = careerDaySPI::setupCareerInstance();        
        return TransformXML::jobListToXml($career);          
    }
    
    /**
     * Allows a user who has completed a career/job to start over, removing all
     * progress. We only allow users to remove a job they have completed, meaning
     * all tasks have to have been completed.
     */
    function resetJob()
    {
        $res = careerDaySPI::resetJob();
        if ($res < 0)
        {
            return TransformXML::codeResultToXML($res);
        }        
        // We need to send back an instance of the job -
        // So create a basic one now ...
        $career = careerDaySPI::setupCareerInstance();        
        return TransformXML::jobListToXml($career);        
    }
    
    /**
     * Allows the flash layer to call to the API, saying, hey someone just read a book
     * (or did some other task) - the API will take care of deciding if the currently
     * logged in user is active in a career that requires the book to be read.
     *
     * @return  string  An xml error an xml task completed message.
     */
    function submitTask()
    {
        $res = careerDaySPI::submitTask();
        if ($res <= 0)
        {
            return TransformXML::codeResultToXML($res);
        }

        return TransformXML::completeTaskXML($res);
    }
    
    /**
     * Allows the user to start on a career path by selecting to do the first task.
     * This will also generate a db entry to track the user's progress.
     *
     * @return string   A job item.
     */
    function startTask()
    {
        $res = careerDaySPI::startTask();
        if ($res <= 0)
        {
            return TransformXML::codeResultToXML($res);
        }
        
        return TransformXML::jobListToXml($res);
    }
    
    /**
     * Used to provide a list of all jobs, both available and unavailalbe
     * and the users status for each job.
     * 
     * @returns xml An xml jobs collection representation
     */
    function getJobsRepresentation()
    {
        $set = careerDaySPI::getJobsList();    
        return TransformXML::jobsListToXML($set);
    }

    /**
     * Used to provide a list of tasks and their status for the supplied job id.
     *
     * @returns xml An xml job tasks item representation
     *                      or an error representation on failure
     */
    function getJobRepresentation()
    {
        $set = careerDaySPI::getJobList();
        if (!is_object($set))
        {
            return TransformXML::codeResultToXML($set);
        }
        
        return TransformXML::jobListToXml($set);
    }
}