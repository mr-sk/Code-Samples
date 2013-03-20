<?php
/**
 * @version    $Id$
 */

/**
 * @see xxx_Workflow
 */
require_once 'xxx/Workflow.php';

/**
 * @see Zend_Http_Client
 */
require_once 'Zend/Http/Client.php';

/**
 */
class xxx_Workflow_Policy_Inspection_Request extends xxx_Workflow
{
    /**
     * Identifier for this Workflow.
     */
    const ID = 'Policy_Inspection_Request';

    /**
     * Description of this Workflow.
     */
    const DESCRIPTION = 'Request an Inspection from xxx and post a reference of the xxx to xxx.';

    /**
     * xxx failure message
     */
    const RESPONSE_FAILURE = 'Failure';
    const RESPONSE_SUCCESS = 'Success';

    /**#@+ Workflow step identifier */
    const STEP_1 = 'JOB_REFERENCE';
    const STEP_2 = 'PREPARE_PROFILE_REQUEST';
    const STEP_3 = 'EXECUTE_PROFILE_REQUEST';
    const STEP_4 = 'PARSE_PROFILE_REQUEST';
    const STEP_5 = 'INSPECTION_REFERENCE';
    /**#@-*/

    /**#@+ Workflow step description */
    const STEP_1_DESC = 'Post a "WorkflowJob" reference to the target policy.';
    const STEP_2_DESC = 'Prepare the ProfileRequest payload and store as DataItems.';
    const STEP_3_DESC = 'Execute a ProfileRequest and cache the response.';
    const STEP_4_DESC = 'Parse the ProfileRequest stored in cache and repare the requst to xxx';
    const STEP_5_DESC = 'Update the Policy to include the a reference to the ProfileRequest and the InspectionId.';
    /**#@-*/

    /**#@+ Xpath */    
    // Policy
    // [REDACTED]
    const XPATH_POLICY_ID              = "/xxx/Identifiers/xxx[@name='xxx']/@value";

    // xxx Response
    // [REDACTED]
    const XPATH_CONFIRMATION           = "/xxx/Response/*[namespace-uri()='http://schemas.xmlsoap.org/soap/envelope/' and local-name()='Envelope']/*[namespace-uri()='http://schemas.xmlsoap.org/soap/envelope/' and local-name()='Body'][1]/*[namespace-uri()='http://xxx.xxx.com/xxx/' and local-name()='ExecuteResponse'][1]/*[namespace-uri()='http://xxx.xxx.com/xxx/' and local-name()='ExecuteResult'][1]/Response[1]/Result[1]/ConfirmationNumber[1]";
    /**#@-*/    

    /**
     * Executes a job based on the job's current state and returns the job after execution.
     *
     * @param xxx_Job $job Job to be executed
     *
     * @return xxx_Job
     */
    public function execute(xxx_Job $job)
    {
        try
        {
            if(xxx_Workflow::JOB_STATUS_ACTIVE != $job->getStatus())
            {
                $job->setStatus(xxx_Workflow::JOB_STATUS_ACTIVE)->save();
            }

            switch(($step = $job->getCurrentStep()))
            {
                case self::STEP_1:
                {
                    if (!$this->postJobReference($job))
                    {
                        break;
                    }
                    $job->setCurrentStep(self::STEP_2)->save();
                }
                case self::STEP_2:
                {
                    if (!$this->prepareProfileRequest($job))
                    {
                        break;
                    }
                    $job->setCurrentStep(self::STEP_3)->save();
                }
                case self::STEP_3:
                {
                    if (!$this->executeProfileRequest($job))
                    {
                        break;
                    }
                    $job->setCurrentStep(self::STEP_4)->save();
                }
                case self::STEP_4:
                {
                    if (!$this->parseProfileRequest($job))
                    {
                        break;
                    }
                    $job->setCurrentStep(self::STEP_5)->save();
                }
                case self::STEP_5:
                {
                    if (!$this->postInspectionReference($job))
                    {
                        break;
                    }
                    $job->complete();
                    break;
                }                  
                default:
                {
                    $this->_fail($job, sprintf('Invalid step "%s" for job "%s"', $step, $job->getId()), TRUE);
                }
            }
        }
        catch(Exception $e)
        {
            $this->_log->crit("Uncaught workflow exception:\n" . print_r($e, TRUE));
        }

        return $job;
    }
        
    /**
     * Returns a description of this Workflow.
     *
     * @return string
     */
    public function getDescription()
    {
        return self::DESCRIPTION;
    }

    /**
     * Returns the identifier for this Workflow.
     *
     * @return string
     */
    public function getId()
    {
        return self::ID;
    }

    /**
     * Returns the initial step for new Jobs executing this Workflow.
     *
     * @return string
     */
    public function getInitialStep()
    {
        return self::STEP_1;
    }
    
    /**
     * Returns a list of steps defined by this workflow.
     *
     * @return array
     */
    public function getSteps()
    {
        return array(
            array(
                'name'        => self::STEP_1,
                'description' => self::STEP_1_DESC
            ),
            array(
                'name'        => self::STEP_2,
                'description' => self::STEP_2_DESC
            ),
            array(
                'name'        => self::STEP_3,
                'description' => self::STEP_3_DESC
            ),
            array(
                'name'        => self::STEP_4,
                'description' => self::STEP_4_DESC
            ),
            array(
                'name'        => self::STEP_5,
                'description' => self::STEP_5_DESC
            )            
        );
    }
    
    /**
     * A helper method for logging
     * 
     * @param   string  $str    An error message
     */
    public function dbg($str)
    {
        $this->_log->debug($str);
    }
        
    /**
     * Extract the required data from the xxx Response, validate
     * and POST a reference to the Profile in xxx.
     *
     * We force that the required fields are not empty, and if they are
     * something has gone terribly wrong and we fail the job.
     *
     * @param xxx_Job $job The job instance
     * @return boolean  FALSE on failure, TRUE otherwise.
     */
    public function postInspectionReference(xxx_Job $job)
    {
        // Convenience
        $diSet    = $this->_wfconf->dataitem;
        $jobId    = $this->getId();
        $jobUri   = sprintf('%s/jobs/%s', $this->_zconf->api->uri, $jobId);
        $policyId = $job->getDataItem($this->_wfconf->dataitem->policyId);

        if(empty($policyId))
        {
            $this->_fail($job, sprintf('FATAL: Undefined policy ID for job "%s"', $jobId), TRUE);
            return FALSE;
        }

        // Data required for the reference POST
        $refTarget        = $this->_services->xxx->resources->inspection_profiles;
        $refType          = $this->_wfconf->reference->type->profile; //profile;
        $profileLocation  = $job->getDataItem($diSet->xxxLocation);
        $profileUUID      = $job->getDataItem($diSet->xxxUUID);
        $createdTimeStamp = $job->getDataItem($diSet->createdTimestamp);
        $code             = $job->getDataItem($diSet->xxx);

        if (empty($refTarget))
        {
            $this->_fail($job, sprintf('FATAL: Undefined refTarget for job "%s"', $jobId), TRUE);
            return FALSE;            
        }
        if (empty($refType))
        {
            $this->_fail($job, sprintf('FATAL: Undefined refType for job "%s"', $jobId), TRUE);
            return FALSE;              
        }
        if (empty($profileLocation))
        {
            $this->_fail($job, sprintf('FATAL: Undefined profileLocation for job "%s"', $jobId), TRUE);
            return FALSE;              
        }
        if (empty($profileUUID))
        {
            $this->_fail($job, sprintf('FATAL: Undefined profileUUID for job "%s"', $jobId), TRUE);
            return FALSE;              
        }
        if (empty($createdTimeStamp))
        {
            $this->_fail($job, sprintf('FATAL: Undefined createdTimeStamp for job "%s"', $jobId), TRUE);
            return FALSE;              
        }
        if (!in_array($code, array(TRUE, FALSE)))
        {
            $this->_fail($job, sprintf('FATAL: Undefined code for job "%s"', $jobId), TRUE);
            return FALSE;                 
        }
        
        // Not required
        $referenceNumber    = $job->getDataItem($diSet->referenceNumber);
        $confirmationNumber = $job->getDataItem($diSet->confirmationNumber);
        $controlNumber      = $job->getDataItem($diSet->controlNumber);
        
        $cachedItems = array('createdTimeStamp'   => $createdTimeStamp,
                             'ReferenceNumber'    => $referenceNumber,
                             'ConfirmationNumber' => $confirmationNumber,
                             'ControlNumber'      => $controlNumber,
                             'Code'               => $code);

        return $this->_postReference($policyId, $refTarget, $profileUUID, $refType, $profileLocation, $cachedItems);        
    }    
    
    /**
     * Parse the ProfileRequest Response body for all the required data.
     * If the UUID or creatTimestamp are missing, we fail the Job. These
     * always should be in the profileRequest, otherwise something went
     * terribly wrong.
     *
     * If xxx returned a failure, we set this in the dataItem and
     * return TRUE. We cannot process beyond this point.
     *
     * If xxx returned success, we then extract all the required data.
     * 
     * @param xxx_Job $job The job instance
     * @return boolean  FALSE on failure, TRUE otherwise.
     */
    public function parseProfileRequest(xxx_Job $job)
    {
        // Convenience
        $diSet    = $this->_wfconf->dataitem;
        $policyId = $job->getDataItem($this->_wfconf->dataitem->policyId);
        $jobId    = $job->getId();

        // Create a simpleXML object or exit this step.
        $xxxXml = $job->getDataItem($this->_wfconf->dataitem->xxxResponse);

        try
        {
            $xmlObj = new SimpleXMLElement($xxxXml);
        }
        catch(Exception $e)
        {
            $this->_log->err(sprintf('xxx Response: %s', $xxxXml));            
            $this->_fail($job, sprintf('FATAL: Unable to fetch valid XML for job "%s"', $jobId), TRUE);
            return FALSE;
        }
        
        // We always get the profileRequest createdTimestamp
        $resultSet = $xmlObj->xpath('/Profile');
        if (!isset($resultSet[0]['createTimestamp']))
        {
            $this->_fail($job, sprintf('FATAL: Unable to fetch createdTimestamp from ProfileRequest for job "%s"', $jobId), TRUE);         
            return FALSE; // Fail, cause something is totally wrong.            
        }
        $job->setDataItem($diSet->createdTimestamp, (string) $resultSet[0]['createTimestamp']);
        
        if (!isset($resultSet[0]['UUID'])) // We always get the UUID as well.
        {
            $this->_fail($job, sprintf('FATAL: Unable to fetch UUID from ProfileRequest for job "%s"', $jobId), TRUE);         
            return FALSE; // Fail, cause something is totally wrong.            
        }
        $job->setDataItem($diSet->xxxUUID, (string) $resultSet[0]['UUID']);        
        
        // If the request failed, we cannot gather the data we need, so mark it as failed and exit this step.
        $resultSet = $xmlObj->xpath(self::XPATH_CODE);
        if (isset($resultSet[0][0]) && $resultSet[0][0] == self::RESPONSE_FAILURE)
        {
            $job->setDataItem($diSet->xxx, self::RESPONSE_FAILURE);
            return TRUE;    
        }
        else
        {   // If the result is not failure AND it not set, now just check if its set at all,
            // and if not, we have a problem, otherwise set it to success. We assume that if
            // it is set and no equal to fail, then it's success.
            if (!isset($resultSet[0][0]))
            {
                $this->logXpathError(self::XPATH_CODE, $policyId, $jobId);
                return FALSE;            
            }
            $job->setDataItem($diSet->xxx, self::RESPONSE_SUCCESS);
        }
        
        // Otherwise, fetch the remaining data we need from the request.
        $resultSet = $xmlObj->xpath(self::XPATH_CONFIRMATION);    
        if (!isset($resultSet[0][0]))
        {
            $this->logXpathError(self::XPATH_CONFIRMATION, $policyId, $jobId);
            return FALSE;            
        }
        $job->setDataItem($diSet->confirmationNumber, (string) $resultSet[0][0]);
        
        $resultSet = $xmlObj->xpath(self::XPATH_CONTROL);    
        if (!isset($resultSet[0][0]))
        {
            $this->logXpathError(self::XPATH_CONTROL, $policyId, $jobId);
            return FALSE;            
        }
        $job->setDataItem($diSet->controlNumber, (string) $resultSet[0][0]);

        $resultSet = $xmlObj->xpath(self::XPATH_REFERENCE);    
        if (!isset($resultSet[0][0]))
        {
            $this->logXpathError(self::XPATH_REFERENCE, $policyId, $jobId);
            return FALSE;            
        }
        $job->setDataItem($diSet->referenceNumber, (string) $resultSet[0][0]);

        return TRUE;
    }
        
    /**
     * Using the stored xxx Payload, we execute a xxx
     * to xxx. If the result is a failure, we return false and will
     * attempt again later.
     *
     * If the response is success, we store the responseBody as a dataItem
     * and return TRUE.
     *
     * @param xxx_Job $job The job instance
     * @return boolean  FALSE on failure, TRUE otherwise.
     */
    public function executeProfileRequest(xxx_Job $job)
    {
        $diSet = $this->_wfconf->dataitem;
        $ixSet = $this->_wfconf->xxx;
        
        require_once 'ICS/Service/xxx.php';
        $profiler = new ICS_Service_xxx($ixSet->uri, $ixSet->user, $ixSet->pass);
        
        // Post a ProfileRequest to the xxx request template.
        $response = $profiler->postProfileRequest($ixSet->templateName,
                                                  $job->getDataItem($diSet->xxxPayload),
                                                  'xxx');
    
        // If xxx returns an error, log and return false
        if($response->isError())
        {
            $this->_log->alert(sprintf('xxx Response Error [Status %s]: %s', $response->getStatus(), $response->getBody()));
            return FALSE;
        }
        
        // Store the response body.
        $job->setDataItem($diSet->xxxResponse, $response->getBody());
        
        /** @see ICS_Service_xxx_Profile */
        require_once 'ICS/Service/xxx/Profile.php';
        // Store the location header
        $job->setDataItem($diSet->xxxLocation, $response->getHeader('Location'));
            
        return TRUE;    
    }
    
    /**
     * Fetches the cached Policy and attempts to xpath all the required
     * data from it. If any of the xpaths fail, we return false. After
     * each successful xpath we store the value in the corresponding
     * dataItem.
     *
     * @param xxx_Job $job The job instance
     * @return boolean  FALSE on failure, TRUE otherwise.
     */
    public function prepareProfileRequest(xxx_Job $job)
    {
        // For convenience.
        $policyId = $job->getDataItem($this->_wfconf->dataitem->policyId);
        $jobId    = $this->getId();
        $diSet    = $this->_wfconf->dataitem;
        
        // Fetch the cached policy (or get a fresh one).
        $uri = $this->_getPolicyUri($policyId);
        $doc = $this->_getDocument($uri);

        // Create a simpleXML object or exit this step.
        try
        {
            $xmlObj = new SimpleXMLElement($doc);
        }
        catch(Exception $e)
        {
            $this->_log->err(sprintf('Unable to load quote XML (%s) for job "%s"', $policyId, $jobId));            
            return FALSE;
        }        

        // Execute all the Xpath ...
        $resultSet = $xmlObj->xpath(self::XPATH_POLICY_ID);
        if (!isset($resultSet[0]['value']))
        {
            $this->logXpathError(self::XPATH_POLICY_ID, $policyId, $jobId);
            return FALSE;            
        }
        $job->setDataItem($diSet->actualPolicyId, (string) $resultSet[0]['value']);
        
        $resultSet = $xmlObj->xpath(self::XPATH_PROPERTY_STREET_NUMBER);
        if (!isset($resultSet[0]['value']))
        {
            $this->logXpathError(self::XPATH_PROPERTY_STREET_NUMBER, $policyId, $jobId);
            return FALSE;            
        }
        $job->setDataItem($diSet->propertyStreetNumber, (string) $resultSet[0]['value']);

        $resultSet = $xmlObj->xpath(self::XPATH_PROPERTY_STREET_NAME);
        if (!isset($resultSet[0]['value']))
        {
            $this->logXpathError(self::XPATH_PROPERTY_STREET_NAME, $policyId, $jobId);
            return FALSE;
        }
        $job->setDataItem($diSet->propertyStreetName, (string) $resultSet[0]['value']);
        
        $resultSet = $xmlObj->xpath(self::XPATH_PROPERTY_STREET_LINE_2);
        if (!isset($resultSet[0]['value']))
        {
            $this->logXpathError(self::XPATH_PROPERTY_STREET_LINE_2, $policyId, $jobId);
            return FALSE;
        }
        $job->setDataItem($diSet->propertyStreetLine2, (string) $resultSet[0]['value']);
        
        $resultSet = $xmlObj->xpath(self::XPATH_PROPERTY_CITY);
        if (!isset($resultSet[0]['value']))
        {
            $this->logXpathError(self::XPATH_PROPERTY_CITY, $policyId, $jobId);
            return FALSE;
        }
        $job->setDataItem($diSet->propertyCity, (string) $resultSet[0]['value']);
        
        $resultSet = $xmlObj->xpath(self::XPATH_PROPERTY_STATE);
        if (!isset($resultSet[0]['value']))
        {
            $this->logXpathError(self::XPATH_PROPERTY_STATE, $policyId, $jobId);
            return FALSE;
        }
        $job->setDataItem($diSet->propertyState, (string) $resultSet[0]['value']);
        
        $resultSet = $xmlObj->xpath(self::XPATH_PROPERTY_ZIP);
        if (!isset($resultSet[0]['value']))
        {
            $this->logXpathError(self::XPATH_PROPERTY_ZIP, $policyId, $jobId);
            return FALSE;
        }
        $job->setDataItem($diSet->propertyZip, (string) $resultSet[0]['value']);
        
        $resultSet = $xmlObj->xpath(self::XPATH_COVERAGE_A);
        if (!isset($resultSet[0]['value']))
        {
            $this->logXpathError(self::XPATH_COVERAGE_A, $policyId, $jobId);
            return FALSE;
        }
        $job->setDataItem($diSet->coverageA, (string) $resultSet[0]['value']);        
        
        $resultSet = $xmlObj->xpath(self::XPATH_INSURED_ADDRESS_1);
        if (!isset($resultSet[0]['value']))
        {
            $this->logXpathError(self::XPATH_INSURED_ADDRESS_1, $policyId, $jobId);
            return FALSE;
        }
        $job->setDataItem($diSet->insuredAddress1, (string) $resultSet[0]['value']);
        
        $resultSet = $xmlObj->xpath(self::XPATH_INSURED_ADDRESS_2);
        if (!isset($resultSet[0]['value']))
        {
            $this->logXpathError(self::XPATH_INSURED_ADDRESS_2, $policyId, $jobId);
            return FALSE;
        }
        $job->setDataItem($diSet->insuredAddress2, (string) $resultSet[0]['value']);
        
        $resultSet = $xmlObj->xpath(self::XPATH_INSURED_CITY);
        if (!isset($resultSet[0]['value']))
        {
            $this->logXpathError(self::XPATH_INSURED_CITY, $policyId, $jobId);
            return FALSE;
        }
        $job->setDataItem($diSet->insuredCity, (string) $resultSet[0]['value']);
        
        $resultSet = $xmlObj->xpath(self::XPATH_INSURED_STATE);
        if (!isset($resultSet[0]['value']))
        {
            $this->logXpathError(self::XPATH_INSURED_STATE, $policyId, $jobId);
            return FALSE;
        }
        $job->setDataItem($diSet->insuredState, (string) $resultSet[0]['value']);
        
        $resultSet = $xmlObj->xpath(self::XPATH_INSURED_ZIP);
        if (!isset($resultSet[0]['value']))
        {
            $this->logXpathError(self::XPATH_INSURED_ZIP, $policyId, $jobId);
            return FALSE;
        }
        $job->setDataItem($diSet->insuredZip, (string) $resultSet[0]['value']);
        
        $resultSet = $xmlObj->xpath(self::XPATH_INSURED_PHONE);
        if (!isset($resultSet[0]['value']))
        {
            $this->logXpathError(self::XPATH_INSURED_PHONE, $policyId, $jobId);
            return FALSE;
        }
        $resultSet[0]['value'] = str_replace('-', '', $resultSet[0]['value']);
        $job->setDataItem($diSet->insuredPhone, (string) $resultSet[0]['value']);
        
        $resultSet = $xmlObj->xpath(self::XPATH_INSURED_FIRST);
        if (!isset($resultSet[0]['value']))
        {
            $this->logXpathError(self::XPATH_INSURED_FIRST, $policyId, $jobId);
            return FALSE;
        }
        $job->setDataItem($diSet->insuredFirst, (string) $resultSet[0]['value']);
        
        $resultSet = $xmlObj->xpath(self::XPATH_INSURED_MIDDLE);
        if (!isset($resultSet[0]['value']))
        {
            $this->logXpathError(self::XPATH_INSURED_MIDDLE, $policyId, $jobId);
            return FALSE;
        }
        $job->setDataItem($diSet->insuredMiddle, (string) $resultSet[0]['value']);
        
        $resultSet = $xmlObj->xpath(self::XPATH_INSURED_LAST);
        if (!isset($resultSet[0]['value']))
        {
            $this->logXpathError(self::XPATH_INSURED_LAST, $policyId, $jobId);
            return FALSE;
        }
        $job->setDataItem($diSet->insuredLast, (string) $resultSet[0]['value']);
        
        $resultSet = $xmlObj->xpath(self::XPATH_EFFECTIVE_DATE);      
        if (!isset($resultSet[0]['value']))
        {
            $this->logXpathError(self::XPATH_EFFECTIVE_DATE, $policyId, $jobId);
            return FALSE;
        }
        $tmpSet  = explode('/', $resultSet[0]['value']);
        $tmpDate = $tmpSet[2] . '-' . $tmpSet[0] . '-' . $tmpSet[1];
        $job->setDataItem($diSet->effectiveDate, $tmpDate);
        
        $resultSet = $xmlObj->xpath(self::XPATH_AGENT_PHONE);
        if (!isset($resultSet[0]['value']))
        {
            $this->logXpathError(self::XPATH_AGENT_PHONE, $policyId, $jobId);
            return FALSE;
        }
        $resultSet[0]['value'] = str_replace('-', '', $resultSet[0]['value']);        
        $job->setDataItem($diSet->agentPhone, (string) $resultSet[0]['value']);
        
        $resultSet = $xmlObj->xpath(self::XPATH_AGENT_ID);
        if (!isset($resultSet[0][0]))
        {
            $this->logXpathError(self::XPATH_AGENT_ID, $policyId, $jobId);
            return FALSE;
        }
        $job->setDataItem($diSet->agentId, (string) $resultSet[0][0]);
        
        $resultSet = $xmlObj->xpath(self::XPATH_AGENT_NAME);
        if (!isset($resultSet[0]['value']))
        {
            $this->logXpathError(self::XPATH_AGENT_NAME, $policyId, $jobId);
            return FALSE;
        }
        $job->setDataItem($diSet->agentName, (string) $resultSet[0]['value']);           
                
        // Build the xxx ProfileRequest (heh sorry)
        // Note, OrderInspectionAccountNumber comes from ixConfig.
        // [REDACTED]

        $job->setDataItem($diSet->xxxPayload, $xml);
        return TRUE;
    }
    
    /**
     * A helper method for logging xpath errors.
     * 
     * @param   string  $constMsg   The xpath const
     * @param   string  $policyId   The policyId
     * @param   string  $jobId      The jobId
     */
    public function logXpathError($constMsg, $policyId, $jobId)
    {
        $this->_log->err(sprintf('Unable to locate (%s) via XPATH in (%s) for job "%s"', $constMsg, $policyId, $jobId));
    }
    
    /**
     * Posts a WorkflowJob Reference to a Policy via xxx.
     *
     * @param xxx_Job $job Job to be referenced
     *
     * @return boolean Success/failure
     */
    public function postJobReference(xxx_Job $job)
    {
        $jobId    = $job->getId();
        $jobUri   = sprintf('%s/jobs/%s', $this->_zconf->api->uri, $jobId);
        $policyId = $job->getDataItem($this->_wfconf->dataitem->policyId);
	
        if(empty($policyId))
        {
            $this->_fail($job, sprintf('FATAL: Undefined policy ID for job "%s"', $jobId), TRUE);
            return FALSE;
        }

        $refTarget   = $this->_services->xxx->resources->references_jobs;
        $refType     = $this->_wfconf->reference->type->workflowJob;
        $cachedItems = array('WorkflowId' => self::ID);

        return $this->_postReference($policyId, $refTarget, $jobId, $refType, $jobUri, $cachedItems);
    }
}