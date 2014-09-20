#include "stdafx.h"
#include "XmlRpc.h"


XmlRpc::XmlRpc(boost::property_tree::ptree& ptree) :
	tree(ptree)
{
	node = "";
}

XmlRpc::XmlRpc(boost::property_tree::ptree& ptree, char* methodName) :
	tree(ptree)
{
	node = "methodCall.";
	create(methodName);
}

XmlRpc::XmlRpc(boost::property_tree::ptree& ptree, std::string source) :
	tree(ptree)
{
	node = "methodResponse.";
	load(source);
}

std::string XmlRpc::getNode(char* n)
{
	return node + std::string(n);
}

std::string XmlRpc::getNode(std::string n) 
{
	return node + n;
}

void XmlRpc::create(char* methodName)
{
	tree.put(getNode("methodName"), methodName);
	tree.put(getNode("params"), "");
}


XmlRpc XmlRpc::operator[](int i)
{
	std::string pnode = "params";
	std::string cnode = ".param";
	bool array = false;
	boost::optional< boost::property_tree::ptree& > c = tree.get_child_optional(getNode(pnode));

	if (!c) {
		array = true;
		pnode = "array.data";
		cnode = ".value";
		if (tree.count(getNode("array")) == 0) tree.put(getNode(pnode), "");
		c = tree.get_child_optional(getNode(pnode));
	}

	boost::property_tree::ptree& child = c.get();
	int idx = child.size();
	while (idx <= i)
	{
		boost::property_tree::ptree arg;
		if (!array) arg.put("value", "");
		tree.add_child(getNode(pnode + cnode), arg);
		idx++;
	}

	idx = -1;
	BOOST_FOREACH(boost::property_tree::ptree::value_type &v, child)
	{
		idx++;
		if (idx != i) continue;
		if (array) return XmlRpc(v.second);
		return XmlRpc(v.second.get_child("value"));
	}

	return *this;
}

XmlRpc XmlRpc::operator[](char* name)
{
	if (tree.count(getNode("struct")) > 0) {
		boost::property_tree::ptree& child = tree.get_child(getNode("struct"));

		BOOST_FOREACH(boost::property_tree::ptree::value_type &v, child)
		{
			if (v.second.get<std::string>("name").compare(name)) continue;
			return XmlRpc(v.second.get_child("value"));
		}
	}
	// If not found, we create the member
	boost::property_tree::ptree member;
	member.put("name", name);
	member.put("value", "");
	return tree.add_child(getNode("struct.member"), member).get_child("value");
}

char* XmlRpc::operator=(char* str)
{
	tree.put<char*>("string", str);
	return str;
}


int XmlRpc::operator=(int i)
{
	tree.put<int>("int", i);
	return i;
}


std::string XmlRpc::getString()
{
	return tree.get<std::string>("string");
}

boost::property_tree::ptree& XmlRpc::getTree() 
{
	return tree;
}
bool XmlRpc::load(std::string& str)
{
	std::stringstream ss;
	ss << str;
	try 
	{
		boost::property_tree::read_xml(ss, tree, boost::property_tree::xml_parser::trim_whitespace);
		tree = tree.front().second;
	}
	catch(...) 
	{
		print("Unable to read the XmlRpc received");
		return false;
	}
	return true;
}

void XmlRpc::addValue(char* value, char* type)
{
	std::string tmp("value.");
	tmp.append(type);
	boost::property_tree::ptree arg;
    arg.put(tmp, value);
	tree.add_child("params.param", arg);
}

boost::property_tree::ptree XmlRpc::getValueInStruct(int structId, char* name)
{
	int idx = 0;
	BOOST_FOREACH(boost::property_tree::ptree::value_type &v, tree.get_child("methodResponse.params"))
	{
		if (idx != structId) continue;
		boost::property_tree::ptree pt = v.second;
		BOOST_FOREACH(boost::property_tree::ptree::value_type &v2, pt.get_child("value.struct"))
		{
			if (v2.second.get<std::string>("name").compare(name) == 0) 
			{
				return v2.second.get_child("value");
			}
			
		}
		idx++;
	}
	return tree;
}

void XmlRpc::toStr(std::string& str) 
{
	std::stringstream ss;
	boost::property_tree::write_xml(ss, tree);
	str.append(ss.str());
}



XmlRpc::~XmlRpc(void) {}
