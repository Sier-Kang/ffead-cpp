/*
	Copyright 2009-2012, Sumeet Chhetri 
  
    Licensed under the Apache License, Version 2.0 (the "License"); 
    you may not use this file except in compliance with the License. 
    You may obtain a copy of the License at 
  
        http://www.apache.org/licenses/LICENSE-2.0 
  
    Unless required by applicable law or agreed to in writing, software 
    distributed under the License is distributed on an "AS IS" BASIS, 
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
    See the License for the specific language governing permissions and 
    limitations under the License.  
*/
/*
 * BeanContext.cpp
 *
 *  Created on: Mar 27, 2010
 *      Author: sumeet
 */

#include "BeanContext.h"

RemoteComponentInt::RemoteComponentInt() {
	// TODO Auto-generated constructor stub

}

RemoteComponentInt::~RemoteComponentInt() {
	// TODO Auto-generated destructor stub
}

void RemoteComponentInt::setBeanContext(const BeanContext& cntxt)
{
	this->cntxt = cntxt;;
}
BeanContext RemoteComponentInt::getContext()
{
	return this->cntxt;
}


BeanContext::BeanContext() {
	port = -1;
	logger = LoggerFactory::getLogger("BeanContext");
}

BeanContext::BeanContext(const std::string& host, const int& port) {
	this->host = host;
	this->port = port;
}

BeanContext::~BeanContext() {
	// TODO Auto-generated destructor stub
}
std::string BeanContext::getHost() const
{
	return host;
}

void BeanContext::setHost(const std::string& host)
{
	this->host = host;
}

int BeanContext::getPort() const
{
	return port;
}

void BeanContext::setPort(const int& port)
{
	this->port = port;
}

void* BeanContext::lookup(const std::string& cmpName)
{
	void *_temp = NULL;
	if(!client.isConnected())
		client.connection(host,port);
	if(client.isConnected())
	{
		Reflector ref;
		std::string classn;
		classn = "Component_"+cmpName+"_Remote";
		ClassInfo clas = ref.getClassInfo(classn);
		args argus;
		Constructor ctor = clas.getConstructor(argus);
		_temp = ref.newInstanceGVP(ctor);
		RemoteComponentInt* intf  = static_cast<RemoteComponentInt*>(_temp);
		intf->setBeanContext(*this);
	}
	return _temp;
}

void* BeanContext::invoke(const std::string& name, std::vector<GenericObject> args, const std::string& bname, const std::string& rettyp)
{
	void* retval = NULL;
	if(client.isConnected())
	{
		XMLSerialize ser;
		std::string argus;
		if(args.size()>0)
		{
			for (unsigned int var = 0; var < args.size(); ++var)
			{
				argus += "<argument type=\""+rettyp+"\">"+ser.serializeUnknown(args.at(var).getPointer(),rettyp)+"</argument>";
			}
		}
		std::string call = "<service name=\""+name+"\" beanName=\""+bname+"\" lang=\"c++\" returnType=\""+rettyp+"\"><args>"+argus+"</args></service>";
		client.sendData(call);
		call = "";
		while((call=client.getData())=="")
		{
		}
		//logger << call << std::flush;
		XmlParser parser("Parser");
		Document doc;
		parser.parse(call, doc);
		Element message = doc.getRootElement();
		if(message.getTagName().find("<return:exception>")==std::string::npos)
		{
			std::string tag = message.getTagName();
			StringUtil::replaceFirst(tag,"return:","");
			message.setTagName(tag);
			call = message.render();
			//logger << call << std::flush;
			retval = ser.unSerializeUnknown(call,tag);
		}
		else
		{
			throw std::runtime_error("Exception occurred");
		}
		//logger << retval << std::flush;
	}
	else
	{
		throw std::runtime_error("Invalid BeanContext");
	}
	return retval;
}
