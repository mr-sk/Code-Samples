<?php
class Profilecache extends Model
{
    const CACHE_DATABASE = 'db';        /* This is defined in the template XML. */
    const CACHE_MEMCACHE = 'memcache';  /* ditto. */
                                          
    const CPT_HASH       = 'RequestHash';      /* Database field names. */ 
    const CPT_RESPONSE   = 'ResponseBody';
    const CPT_CREATED    = 'CreatedDate';

    const SP_ADD_CACHED_PROFILE    = 'spAddCachedTransaction'; /* Stored procs. */
    const SP_GET_CACHED_PROFILE    = 'spGetCachedTransaction';
    const SP_IS_DUPLICATE          = 'spIsHashDuplicate';
    const SP_UPDATE_CACHED_PROFILE = 'spUpdateCachedTransaction';
    
    private $_algo              = 'md5';       /* Our algorithm of choice. */
    private $_hash              = null;        /* Hash starts empty. */
    private $_cacheType         = null;        /* No cache type defined. */
    private $_cachedProfileId   = null;
    private $_cachedProfileDate = null; 
    
    /* Cache types we currently support. */
    private $_cacheTypes = array(self::CACHE_DATABASE => self::CACHE_DATABASE,
                                 self::CACHE_MEMCACHE => self::CACHE_MEMCACHE);
    
    /**
     * @param   string    $cacheType    The type of cache we are using for this Request.
     * @return  Exception               If the cache type is incorrect, thrown an Exception.  
     */
    public function __construct($cacheType)
    {
        parent::__construct();
        
        if (!isset($this->_cacheTypes[strtolower($cacheType)]))
        {
            throw new Exception(sprintf("Unknown cacheType (%s). Support cacheTypes are (%s)",
                                $cacheType, $this->displaySupportedCacheTypes()));
        }
        
        $this->setCacheType(strtolower($cacheType));
    }
    
    
    /**
     * @return  string  A formatted message of the support cacheTypes.
     */
    private function displaySupportedCacheTypes()
    {
        $tmpStr = '';
        foreach($this->_cacheTypes as $type)
        {
            $tmpStr .= " $type ";
        }
        return $tmpStr;
    }
    
    
    /**
     * @param   string  A supported cacheType.
     */
    private function setCacheType($cacheType)
    {
        $this->_cacheType = $cacheType;
    }
    
    
    /**
     * @return  string  The current cacheType.
     */
    private function getCacheType()
    {
        return $this->_cacheType;    
    }
   
    
    /**
     * @param   string  The computed hash.
     */
    private function setHash($hash)
    {
        $this->_hash = $hash;    
    }
    
    
    /**
     * @return  string The hash.
     */
    public function getHash()
    {
        return $this->_hash;
    }
    
    public function getCachedTransactionResponse()
    {
        if ($this->_cachedProfileId == null) { throw new Exception('cachedProfileId null'); }
        return $this->_cachedProfileId;    
    }
    
    
    public function getCachedTransactionOriginalDate()
    {
       if ($this->_cachedProfileDate == null) { throw new Exception('cachedProfileDate null'); }           
       return $this->_cachedProfileDate;
    }
    
    
    /**
     * @param   string    $hashBody     The body of the request
     * @param   string    $hashHeader   (optional) The header
     * @param   string    $hashUri      (optional) The URI of the request
     * @param   string    $hashAuth     (optional) Auth
     *                                          
     * @return  Exception   If the COMPUTED hash is empty.
     */
    public function computeHash($hashBody, $hashHeader, $hashUri, $hashAuth)
    {
        $localHash = '';
        $localHash = $hashBody  . $hashHeader  . $hashUri  . $hashAuth;
        if ($localHash == '')
        {
            throw new Exception('Computed hash is empty');
        }
        
        $this->setHash(hash($this->_algo, $localHash));
    }
    
    
    /**
     * Attempt to set the transactionCache.
     *  If the argument is empty, throw an execption.
     *  If the hash is null, throw an exception.
     *  If the cacheType is unknown, throw an exception.
     *
     * Call the appropriate caching method.
     * 
     * @param   string  $profileId The profile Id.
     * @return  mixed   Could be FALSE, if the request to the cache failes, or an array/string
     *                  of the CachedProfile table result.
     *
     */
    public function setTransactionCache($profileId)
    {
        if ($this->getHash() == null) { throw new Exception('Hash is null'); }
        if ($profileId == '') { throw new Exception('setTransactionCache profileId is empty'); }
        switch($this->getCacheType())
        {
            case self::CACHE_DATABASE:
                return $this->setProfileCacheToDB($profileId);
            break;
        
            case self::CACHE_MEMCACHE:
                return $this->setProfileCacheToMemcache($profileId);
            break;

            default:
                throw new Exception(sprintf("setProfileCache: unknown type (%s)", $this->getCacheType()));
        }
    }
    
    
    /**
     * Save the hash and response (serialized in caller) to the database.
     * This handles an UPDATE or INSERT.
     * 
     * @param string    $profileId  (Serialized) The profile Id.
     */
    private function setProfileCacheToDB($profileId)
    {
        /* If the hash already exists in the database, we want to update it.
           Otherwise, we create a new entry.
        */
        $bindSet = array(self::CPT_HASH      => $this->getHash(),
                         self::CPT_RESPONSE => $profileId);
        
        $result = $this->executeStoreProc(self::SP_IS_DUPLICATE, array(self::CPT_HASH => $this->getHash()));
        if (isset($result[0][self::CPT_HASH]))
        {
            $request = $this->executeStoreProc(self::SP_UPDATE_CACHED_PROFILE, $bindSet);     
        }
        else
        {
            $request = $this->executeStoreProc(self::SP_ADD_CACHED_PROFILE, $bindSet);
        }
    }
    
    
    /**
     * This method is built to keep a clean interface. Essentially we could use
     * polymorphism here, but its never going to be bigger than these two cache
     * methods, so instead you call this method and it takes care of handling
     * what backend cache to use.
     * 
     * @param   int $threshold  The number of days this profile stays valid for.
     * @return  mixed           The SQL result.
     */
    public function getProfileCache($threshold)
    {
        if ($this->getHash() == null) { throw new Exception('Hash is null'); }
        switch($this->getCacheType())
        {
            case self::CACHE_DATABASE:
                return $this->getProfileCacheFromDB($threshold);
            break;
        
            case self::CACHE_MEMCACHE:
                return $this->getProfileCacheFromMemcache();
            break;
        
            default:
                throw new Exception(sprintf("getProfileCache: unknown type (%s)", $this->getCacheType()));
        }
    }
    
    
    /**
     * @param   int $threshold   Should be in hours.
     * @return  bool             FALSE is fetching from the db failes or required items are not set
     *                           TRUE otherwise.
     */
    private function getProfileCacheFromDB($threshold)
    {
		$request = $this->executeStoreProc(self::SP_GET_CACHED_PROFILE, array(self::CPT_HASH => $this->getHash()));
        if (empty($request)) { return FALSE; } 
        else
        {
            if (isset($request[0][self::CPT_CREATED]))
            {
                /* lastUpdateDate and rightNow are in SECONDS. */
                $lastUpdateDate = date_format($request[0][self::CPT_CREATED], 'U'); 
                $rightNow       = date('U');

                /* If (The time right now) is LESS THAN
                        (the last time this cache was updated + threshold (adjusted to seconds))
                */
                if ($rightNow < ($lastUpdateDate + ($threshold * 3600))) // 3600 = # of secs in hour
                {
                    if (isset($request[0][self::CPT_RESPONSE]) )
                    {
                        $this->_cachedProfileId   = $request[0][self::CPT_RESPONSE];
                        $this->_cachedProfileDate = $request[0][self::CPT_CREATED];
                        return TRUE;
                    }
                }
            }
            return FALSE;
        }
    }
    
    
    /** This is not implemented! */
    private function getProfileCacheFromMemcache()
    {
        assert(!"Not implemented.");
    }
    
    /** This is not implemented! */
    private function setProfileCacheToMemcache()
    {
        assert(!"Not implemented.");
    }    
}