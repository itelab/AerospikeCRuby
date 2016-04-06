class AerospikeC::ErrorCodes
  attr_reader :code, :msg

  AS_PROTO_RESULT_OK = 0
  AS_PROTO_RESULT_FAIL_UNKNOWN = 1
  AS_PROTO_RESULT_FAIL_NOTFOUND = 2
  AS_PROTO_RESULT_FAIL_GENERATION = 3
  AS_PROTO_RESULT_FAIL_PARAMETER = 4
  AS_PROTO_RESULT_FAIL_RECORD_EXISTS = 5
  AS_PROTO_RESULT_FAIL_BIN_EXISTS = 6
  AS_PROTO_RESULT_FAIL_CLUSTER_KEY_MISMATCH = 7
  AS_PROTO_RESULT_FAIL_PARTITION_OUT_OF_SPACE = 8
  AS_PROTO_RESULT_FAIL_TIMEOUT = 9
  AS_PROTO_RESULT_FAIL_NO_XDR = 10
  AS_PROTO_RESULT_FAIL_UNAVAILABLE = 11
  AS_PROTO_RESULT_FAIL_INCOMPATIBLE_TYPE = 12
  AS_PROTO_RESULT_FAIL_RECORD_TOO_BIG = 13
  AS_PROTO_RESULT_FAIL_KEY_BUSY = 14
  AS_PROTO_RESULT_FAIL_SCAN_ABORT = 15
  AS_PROTO_RESULT_FAIL_UNSUPPORTED_FEATURE = 16
  AS_PROTO_RESULT_FAIL_BIN_NOT_FOUND = 17
  AS_PROTO_RESULT_FAIL_DEVICE_OVERLOAD = 18
  AS_PROTO_RESULT_FAIL_KEY_MISMATCH = 19
  AS_PROTO_RESULT_FAIL_NAMESPACE = 20
  AS_PROTO_RESULT_FAIL_BIN_NAME = 21
  AS_PROTO_RESULT_FAIL_FORBIDDEN = 22
  AS_SEC_ERR_OK_LAST = 50
  AS_SEC_ERR_NOT_SUPPORTED = 51
  AS_SEC_ERR_NOT_ENABLED = 52
  AS_SEC_ERR_SCHEME = 53
  AS_SEC_ERR_COMMAND = 54
  AS_SEC_ERR_FIELD = 55
  AS_SEC_ERR_STATE = 56
  AS_SEC_ERR_USER = 60
  AS_SEC_ERR_USER_EXISTS = 61
  AS_SEC_ERR_PASSWORD = 62
  AS_SEC_ERR_EXPIRED_PASSWORD = 63
  AS_SEC_ERR_FORBIDDEN_PASSWORD = 64
  AS_SEC_ERR_CREDENTIAL = 65
  AS_SEC_ERR_ROLE = 70
  AS_SEC_ERR_ROLE_EXISTS = 71
  AS_SEC_ERR_PRIVILEGE = 72
  AS_SEC_ERR_AUTHENTICATED = 80
  AS_SEC_ERR_ROLE_VIOLATION = 81
  AS_PROTO_RESULT_FAIL_UDF_EXECUTION = 100
  AS_PROTO_RESULT_FAIL_COLLECTION_ITEM_NOT_FOUND = 125
  AS_PROTO_RESULT_FAIL_BATCH_DISABLED = 150
  AS_PROTO_RESULT_FAIL_BATCH_MAX_REQUESTS = 151
  AS_PROTO_RESULT_FAIL_BATCH_QUEUES_FULL = 152
  AS_PROTO_RESULT_FAIL_GEO_INVALID_GEOJSON = 160
  AS_PROTO_RESULT_FAIL_INDEX_FOUND = 200
  AS_PROTO_RESULT_FAIL_INDEX_NOT_FOUND = 201
  AS_PROTO_RESULT_FAIL_INDEX_OOM = 202
  AS_PROTO_RESULT_FAIL_INDEX_NOTREADABLE = 203
  AS_PROTO_RESULT_FAIL_INDEX_GENERIC = 204
  AS_PROTO_RESULT_FAIL_INDEX_NAME_MAXLEN = 205
  AS_PROTO_RESULT_FAIL_INDEX_MAXCOUNT = 206
  AS_PROTO_RESULT_FAIL_QUERY_USERABORT = 210
  AS_PROTO_RESULT_FAIL_QUERY_QUEUEFULL = 211
  AS_PROTO_RESULT_FAIL_QUERY_TIMEOUT = 212
  AS_PROTO_RESULT_FAIL_QUERY_CBERROR = 213
  AS_PROTO_RESULT_FAIL_QUERY_NETIO_ERR = 214
  AS_PROTO_RESULT_FAIL_QUERY_DUPLICATE = 215


  #----------------------------------------------------------------------------
  ## @brief      Init Err
  ##
  ## @param      code  error code
  ##
  ##
  def initialize(code)
    @code = code
    @msg = message_from_code
  end


  #----------------------------------------------------------------------------
  ## @brief      Error as json
  ##
  ## @param      options  as_json options parameter
  ##
  ## @return     error hash for json
  ##
  def as_json(options = {})
    {code: @code, msg: @msg}
  end


  #----------------------------------------------------------------------------
  ##                            private
  ##
  private
    def message_from_code
      case @code
      when AS_PROTO_RESULT_FAIL_UNKNOWN
        "Unknown server error."
      when AS_PROTO_RESULT_FAIL_NOTFOUND
        "No record is found with the specified namespace/set/key combination. Check the correct namesapce/set/key is passed in."
      when AS_PROTO_RESULT_FAIL_GENERATION
        "Attempt to modify a record with unexpected generation. This happens on a read-modify-write situation where concurrent write requests collide and only one wins."
      when AS_PROTO_RESULT_FAIL_PARAMETER
        "Illegal parameter sent from client. Check client parameters and verify each is supported by current server version."
      when AS_PROTO_RESULT_FAIL_RECORD_EXISTS
        "For write requests which specify 'CREATE_ONLY', request fail because record already exists."
      when AS_PROTO_RESULT_FAIL_BIN_EXISTS
        "(future) For future write requests which specify 'BIN_CREATE_ONLY', request fail because any of the bin already exists."
      when AS_PROTO_RESULT_FAIL_CLUSTER_KEY_MISMATCH
        "On scan requests, the scan terminates because cluster is in migration. Only occur when client requested 'fail_on_cluster_change' policy on scan."
      when AS_PROTO_RESULT_FAIL_PARTITION_OUT_OF_SPACE
        "Occurs when the stop-write is reached due to memory or storage capacity. Namespace no longer can accept write requests."
      when AS_PROTO_RESULT_FAIL_TIMEOUT
        "Request was not completed during the allocated time, thus aborted."
      when AS_PROTO_RESULT_FAIL_NO_XDR
        "Write request is rejected because XDR is not running. Only occur when XDR configuration xdr-stop-writes-noxdr is on."
      when AS_PROTO_RESULT_FAIL_UNAVAILABLE
        "Server is not accepting requests. Occur during single node on a quick restart to join existing cluster."
      when AS_PROTO_RESULT_FAIL_INCOMPATIBLE_TYPE
        "Operation is not allowed due to data type or namespace configuration incompatibility. For example, append to a float data type, or insert a non-integer when namespace is configured as data-in-index."
      when AS_PROTO_RESULT_FAIL_RECORD_TOO_BIG
        "Attempt to write a record whose size is bigger than the configured write-block-size."
      when AS_PROTO_RESULT_FAIL_KEY_BUSY
        "Too many concurrent operations (> transaction-pending-limit) on the same record."
      when AS_PROTO_RESULT_FAIL_SCAN_ABORT
        "Scan aborted by user on Server."
      when AS_PROTO_RESULT_FAIL_UNSUPPORTED_FEATURE
        "This feature currently is not supported."
      when AS_PROTO_RESULT_FAIL_BIN_NOT_FOUND
        "(future) For future write requests which specify 'REPLACE_ONLY', request fail because specified bin name does not exist in record."
      when AS_PROTO_RESULT_FAIL_DEVICE_OVERLOAD
        "Write request is rejected because storage device is not keeping up."
      when AS_PROTO_RESULT_FAIL_KEY_MISMATCH
        "For update request on records which has key stored, the incoming key does not match the existing stored key. This indicates a RIPEMD160 key collision, and has never happened."
      when AS_PROTO_RESULT_FAIL_NAMESPACE
        "The passed in namespace does not exist on cluster, or no namespace parameter is passed in on request."
      when AS_PROTO_RESULT_FAIL_BIN_NAME
        "Bin name length greater than 14 characters, or maximum number of unique bin names are exceeded.  "
      when AS_PROTO_RESULT_FAIL_FORBIDDEN
        "Operation not allowed at this time. For writes, the set is in the middle of being deleted, or the set's stop-write is reached; For scan, too many concurrent scan jobs (> scan-max-active); For XDR-ed cluster, fail writes which are not replicated from another datacenter."
      when AS_SEC_ERR_OK_LAST
        "End of security response."
      when AS_SEC_ERR_NOT_SUPPORTED
        "Security functionality not supported by connected server."
      when AS_SEC_ERR_NOT_ENABLED
        "Security functionality not enabled by connected server."
      when AS_SEC_ERR_SCHEME
        "Security scheme not supported."
      when AS_SEC_ERR_COMMAND
        "Unrecognized security command."
      when AS_SEC_ERR_FIELD
        "Field is not valid"
      when AS_SEC_ERR_STATE
        " Security protocol not followed."
      when AS_SEC_ERR_USER
        "No user supplied or unknown user."
      when AS_SEC_ERR_USER_EXISTS
        "User already exists."
      when AS_SEC_ERR_PASSWORD
        "Password does not exists or not recognized."
      when AS_SEC_ERR_EXPIRED_PASSWORD
        "Expired password."
      when AS_SEC_ERR_FORBIDDEN_PASSWORD
        "Forbidden password (e.g. recently used)."
      when AS_SEC_ERR_CREDENTIAL
        "Invalid credential or credential does not exist."
      when AS_SEC_ERR_ROLE
        "No role(s) or unknown role(s)."
      when AS_SEC_ERR_ROLE_EXISTS
        "Role already exists. "
      when AS_SEC_ERR_PRIVILEGE
        "Privilege is invalid."
      when AS_SEC_ERR_AUTHENTICATED
        "User must be authenticated before performing database operations."
      when AS_SEC_ERR_ROLE_VIOLATION
        "User does not possess the required role to perform the database operation."
      when AS_PROTO_RESULT_FAIL_UDF_EXECUTION
        "A user defined function failed to execute."
      when AS_PROTO_RESULT_FAIL_COLLECTION_ITEM_NOT_FOUND
        "The requested LDT item not found."
      when AS_PROTO_RESULT_FAIL_BATCH_DISABLED
        "Batch functionality has been disabled by configuring the batch-index-thread=0. "
      when AS_PROTO_RESULT_FAIL_BATCH_MAX_REQUESTS
        "Batch max requests has been exceeded.  "
      when AS_PROTO_RESULT_FAIL_BATCH_QUEUES_FULL
        "All batch queues are full. "
      when AS_PROTO_RESULT_FAIL_GEO_INVALID_GEOJSON
        "GeoJSON is malformed or not supported."
      when AS_PROTO_RESULT_FAIL_INDEX_FOUND
        "Secondary index already exists."
      when AS_PROTO_RESULT_FAIL_INDEX_NOT_FOUND
        "Secondary index does not exist."
      when AS_PROTO_RESULT_FAIL_INDEX_OOM
        "Secondary index memory space exceeded."
      when AS_PROTO_RESULT_FAIL_INDEX_NOTREADABLE
        "Secondary index not available for query. Occurs when indexing creation has not finished."
      when AS_PROTO_RESULT_FAIL_INDEX_GENERIC
        "Generic secondary index error."
      when AS_PROTO_RESULT_FAIL_INDEX_NAME_MAXLEN
        "Index name maximun length exceeded."
      when AS_PROTO_RESULT_FAIL_INDEX_MAXCOUNT
        "Maximum number of indicies exceeded."
      when AS_PROTO_RESULT_FAIL_QUERY_USERABORT
        "Secondary index query aborted."
      when AS_PROTO_RESULT_FAIL_QUERY_QUEUEFULL
        "Secondary index queue full."
      when AS_PROTO_RESULT_FAIL_QUERY_TIMEOUT
        "Secondary index query timed out on server. "
      when AS_PROTO_RESULT_FAIL_QUERY_CBERROR
        "Generic query error."
      when AS_PROTO_RESULT_FAIL_QUERY_DUPLICATE
        "Internal error."
      end
    end
end


