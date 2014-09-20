#pragma once
class XmlRpc
{
public:
	XmlRpc(boost::property_tree::ptree& ptree, char* methodName);
	XmlRpc(boost::property_tree::ptree& ptree, std::string source);
	XmlRpc(boost::property_tree::ptree& ptree);
	

	~XmlRpc(void);
	
	void create(char* methodName);
	bool load(std::string& str);
	void addValue(char* value, char* type = "string");
	boost::property_tree::ptree getValueInStruct(int structId, char* name);
	void toStr(std::string& str);
	XmlRpc operator[](int i);
	XmlRpc operator[](char* v);
	char* operator=(char* str);
	int operator=(int i);
	std::string getString();
	boost::property_tree::ptree& getTree();

private:
	std::string getNode(char* n);
	std::string getNode(std::string n);
	boost::property_tree::ptree& tree;
	bool created;
	std::string node;
};

