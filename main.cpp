#include <glib.h>
#include <string>
#include <luna-service2/lunaservice.h>
#include <PmLog.h>
#include <pbnjson.hpp>

static PmLogContext getPmLogContext()
{
    static PmLogContext s_context = 0;
    if (0 == s_context)
    {
        PmLogGetContext("testDB", &s_context);
    }
    return s_context;
}

static pbnjson::JValue convertStringToJson(const char *rawData)
{
    pbnjson::JInput input(rawData);
    pbnjson::JSchema schema = pbnjson::JSchemaFragment("{}");
    pbnjson::JDomParser parser;
    if (!parser.parse(input, schema))
    {
        return pbnjson::JValue();
    }
    return parser.getDom();
}

static std::string convertJsonToString(const pbnjson::JValue json)
{
    return pbnjson::JGenerator::serialize(json, pbnjson::JSchemaFragment("{}"));
}



static bool cbGetValue(LSHandle *sh, LSMessage *msg, void *user_data)
{
    LSError lserror;
    LSErrorInit(&lserror);


    pbnjson::JValue response = convertStringToJson(LSMessageGetPayload(msg));
   // bool successCallback = response["returnValue"].asBool();

	if(!LSMessageReply(sh, message, reply.response().c_str(), &lserror))
    	{
        	PmLogError(getPmLogContext(), "HANDLE_HELLO", 0, "Message reply error!!");
       	        LSErrorPrint(&lserror, stdout);

             return false;
       }
    

    return true;
}


int main(int argc, char* argv[])
{
    PmLogInfo(getPmLogContext(), "SERVICE_MAIN", 0, "start com.webos.test");

    LSError lserror;
    LSErrorInit(&lserror);

    GMainLoop* mainLoop = g_main_loop_new(nullptr, false);
    LSHandle *m_handle = nullptr;

    if(!LSRegister("com.webos.test", &m_handle, &lserror))
    {
        PmLogError(getPmLogContext(), "LS_REGISTER", 0, "Unable to register to luna-bus");
        LSErrorPrint(&lserror, stdout);

        return false;
    }

  /*  if (!LSRegisterCategory(m_handle, "/", serviceMethods, NULL, NULL, &lserror))
    {
        PmLogError(getPmLogContext(), "LS_REGISTER", 0, "Unable to register category and method");
        LSErrorPrint(&lserror, stdout);

        return false;
    }*/

    if(!LSGmainAttach(m_handle, mainLoop, &lserror))
    {
        PmLogError(getPmLogContext(), "LS_REGISTER", 0, "Unable to attach service");
        LSErrorPrint(&lserror, stdout);

        return false;
    }

    std::string payload ="{ \"query\":{ \"from\":\"com.webos.employee:1\" } }";
    if (!LSCall(m_handle,
                "luna://com.webos.service.db/find",payload,
                cbGetValue,
                NULL,
                NULL,
                &lserror))
    {
        PmLogError(getPmLogContext(), "LSCALL_GETTIME", 0, "Cannot call getTime");
        LSErrorPrint(&lserror, stderr);
    }

    g_main_loop_run(mainLoop);

    if(!LSUnregister(m_handle, &lserror))
    {
        PmLogError(getPmLogContext(), "LS_REGISTER", 0, "Unable to unregister service");
        LSErrorPrint(&lserror, stdout);

        return false;
    }

    g_main_loop_unref(mainLoop);
    mainLoop = nullptr;

    return 0;
}