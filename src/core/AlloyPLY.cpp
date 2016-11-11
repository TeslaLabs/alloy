/*
 * Copyright(C) 2015, Blake C. Lucas, Ph.D. (img.science@gmail.com)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include "AlloyPLY.h"
#include "AlloyFileUtil.h"
#include "AlloyCommon.h"
#include <string>
#include <list>
#include <map>
#include <fstream>
#include <iostream>
#include <stddef.h>
#include <memory>
using namespace std;
namespace aly {
namespace ply {

/******************************************************************************
 Find an element from the element list of a given PLY object.

 Entry:
 plyfile - file id for PLY file
 element - name of element we're looking for

 Exit:
 returns the element, or nullptr if not found
 ******************************************************************************/

PlyElement* PLYReaderWriter::findElement(const std::string& element) {
	int i = 0;
	for (i = 0; i < (int) plyFile->elems.size(); i++) {
		if (element == plyFile->elems[i]->name) {
			return plyFile->elems[i].get();
		}
	}
	return nullptr;
}

PlyFile* PLYReaderWriter::write(const std::vector<std::string>& elem_names,
		const FileFormat& file_type) {
	int i;
	PlyFile *plyFile;
	PlyElement *elem;
	/* create a record for this object */

	plyFile = new PlyFile();
	plyFile->file_type = file_type;
	plyFile->version = 1.0;

	/* tuck aside the names of the elements */

	plyFile->elems.resize(elem_names.size());
	for (i = 0; i < (int) elem_names.size(); i++) {
		elem = (PlyElement *) new PlyElement();
		plyFile->elems[i] = shared_ptr<PlyElement>(elem);
		elem->name = elem_names[i];
		elem->num = 0;
	}
	return plyFile;
}
void PLYReaderWriter::openForReading(const std::string& fileName) {
	std::vector<string> elemNames;
	openForReading(fileName, &elemNames);
	plyFile->elemNames = elemNames;
}
void PLYReaderWriter::openForWriting(const std::string& fileName,
		const std::vector<std::string>& elem_names,
		const FileFormat& file_type) {
	out.open(fileName.c_str(), ios::out | ios::binary);
	if (!out.is_open()) {
		return throw std::runtime_error(
				aly::MakeString() << "Could not open " << fileName
						<< " for writing.");
	}
	PlyFile* file = write(elem_names, file_type);
	if (file == nullptr) {
		return throw std::runtime_error(
				aly::MakeString() << "Could not initialize " << fileName
						<< " for writing.");
	}
	plyFile = std::unique_ptr<PlyFile>(file);
}

/******************************************************************************
 Add an element to a PLY file descriptor.

 Entry:
 plyFile - PLY file descriptor
 words   - list of words describing the element
 nwords  - number of words in the list
 ******************************************************************************/

void PLYReaderWriter::addElement(const std::vector<std::string>& words) {
	PlyElement *elem;
	/* create the new element */
	elem = new PlyElement();
	elem->name = words[1];
	elem->num = atoi(words[2].c_str());
	plyFile->elems.push_back(std::shared_ptr<PlyElement>(elem));
}

/******************************************************************************
 Return the type of a property, given the name of the property.

 Entry:
 name - name of property type

 Exit:
 returns integer code for property, or 0 if not found
 ******************************************************************************/

DataType PLYReaderWriter::getPropType(const std::string& type_name) {
	int i;
	/* try to match the type name */
	for (i = static_cast<int>(DataType::StartType) + 1;
			i < static_cast<int>(DataType::EndType); i++)
		if (type_name == property_type_names[i])
			return static_cast<DataType>(i);

	for (i = static_cast<int>(DataType::StartType) + 1;
			i < static_cast<int>(DataType::EndType); i++)
		if (type_name == old_property_type_names[i])
			return static_cast<DataType>(i);
	/* if we get here, we didn't find the type */
	return DataType::StartType;
}

/******************************************************************************
 Add a property to a PLY file descriptor.

 Entry:
 plyFile - PLY file descriptor
 words   - list of words describing the property
 nwords  - number of words in the list
 ******************************************************************************/
void PLYReaderWriter::addProperty(const std::vector<std::string>& words) {
	PlyProperty *prop;

	/* create the new property */
	prop = new PlyProperty();
	prop->external_type = DataType::StartType;
	if (words[1] == "list") { /* list */
		prop->count_external = getPropType(words[2]);
		prop->external_type = getPropType(words[3]);
		prop->name = words[4];
		prop->is_list = SectionType::List;
	} else if (words[1] == "string") { /* string */
		prop->count_external = DataType::Int8;
		prop->external_type = DataType::Int8;
		prop->name = words[2];
		prop->is_list = SectionType::String;
	} else { /* scalar */
		prop->external_type = getPropType(words[1]);
		prop->name = words[2];
		prop->is_list = SectionType::Scalar;
	}
	PlyElement *elem = plyFile->elems.back().get();
	elem->props.push_back(std::shared_ptr<PlyProperty>(prop));
}

/******************************************************************************
 Add a comment to a PLY file descriptor.

 Entry:
 plyFile - PLY file descriptor
 line    - line containing comment
 ******************************************************************************/

void PLYReaderWriter::addComment(const std::string& line) {
	int i = (int)line.find("comment");
	if (i >= 0) {
		i = i + (int)std::string("comment").size();
	}
	else {
		i = 0;
	}
	while (line[i] == ' ' || line[i] == '\t')
		i++;

	std::string str = line;
	appendComment(str.substr(i));
}

/******************************************************************************
 Add a some object information to a PLY file descriptor.

 Entry:
 plyFile - PLY file descriptor
 line    - line containing text info
 ******************************************************************************/

void PLYReaderWriter::addObjInfo(const std::string& line) {
	int i = (int)line.find("obj_info");
	if (i >= 0) {
		i = i + (int)std::string("obj_info").size();
	}
	else {
		i = 0;
	}
	while (line[i] == ' ' || line[i] == '\t')
		i++;
	appendObjInfo(line.substr(i));
}
void PLYReaderWriter::elementLayout(const std::string& elem_name, int nelems,
		int nprops, const std::vector<PlyProperty>& prop_list)

		/******************************************************************************/
		/*
		 Describe an element, including its properties and how many will be written
		 to the file.

		 Entry:
		 plyFile   - file identifier
		 elem_name - name of element that information is being specified about
		 nelems    - number of elements of this type to be written
		 nprops    - number of properties contained in the element
		 prop_list - list of properties
		 */
		{
	int i;
	PlyElement *elem;
	PlyProperty *prop;
	/* look for appropriate element */
	elem = findElement(elem_name);
	if (elem == nullptr) {
		throw std::runtime_error(
				aly::MakeString() << "element_layout_ply: can't find element "
						<< elem_name);
	}
	elem->num = nelems;
	elem->props = std::vector<std::shared_ptr<PlyProperty>>(nprops);
	elem->store_prop = std::vector<char>(nprops);
	for (i = 0; i < nprops; i++) {
		prop = new PlyProperty();
		*prop = prop_list[i];
		elem->props[i] = std::shared_ptr<PlyProperty>(prop);
		elem->store_prop[i] = NAMED_PROP;
	}
}
/******************************************************************************/

void PLYReaderWriter::describeProperty(const std::string& elem_name,
		const PlyProperty *prop)

		/******************************************************************************/
		/*
		 Describe a property of an element.

		 Entry:
		 plyFile   - file identifier
		 elem_name - name of element that information is being specified about
		 prop      - the new property
		 */
		{
	PlyElement *elem;
	PlyProperty *elem_prop;
	/* look for appropriate element */
	elem = findElement(elem_name);
	if (elem == nullptr) {
		throw std::runtime_error(
				aly::MakeString() << "describeProperty: can't find element "
						<< elem_name);
	}
	elem_prop = new PlyProperty();
	*elem_prop = *prop;
	elem->props.push_back(std::shared_ptr<PlyProperty>(elem_prop));
	elem->store_prop.push_back(NAMED_PROP);
}

/******************************************************************************/

void PLYReaderWriter::elementCount(const std::string& elem_name, int nelems)
/******************************************************************************/
/*
 State how many of a given element will be written.

 Entry:
 plyFile   - file identifier
 elem_name - name of element that information is being specified about
 nelems    - number of elements of this type to be written
 */
{
	PlyElement *elem;
	elem = findElement(elem_name);
	if (elem == nullptr) {
		throw runtime_error(
				aly::MakeString() << "elementCount: can't find element "
						<< elem_name);
	}
	elem->num = nelems;
}

/******************************************************************************
 Signal that we've described everything a PLY file's header and that the
 header should be written to the file.

 Entry:
 plyFile - file identifier
 ******************************************************************************/

/******************************************************************************/

void PLYReaderWriter::headerComplete() {
	int i, j;
	PlyElement *elem;
	PlyProperty *prop;
	out << "ply\n";
	switch (plyFile->file_type) {
	case FileFormat::ASCII:
		out << "format ascii 1.0\n";
		break;
	case FileFormat::BINARY_BE:
		out << "format binary_big_endian 1.0\n";
		break;
	case FileFormat::BINARY_LE:
		out << "format binary_little_endian 1.0\n";
		break;
	default:
		throw std::runtime_error(
				aly::MakeString() << "headerComplete: bad file type = "
						<< plyFile->file_type);
	}
	/* write out the comments */
	for (i = 0; i < (int) plyFile->comments.size(); i++)
		out << "comment " << plyFile->comments[i] << "\n";
	/* write out object information */
	for (i = 0; i < (int) plyFile->obj_info.size(); i++)
		out << "obj_info " << plyFile->obj_info[i] << "\n";
	/* write out information about each element */
	for (i = 0; i < (int) plyFile->elems.size(); i++) {
		elem = plyFile->elems[i].get();
		out << "element " << elem->name << " " << elem->num << "\n";
		/* write out each property */
		for (j = 0; j < (int) elem->props.size(); j++) {
			prop = elem->props[j].get();
			if (prop->is_list == SectionType::List) {
				out << "property list ";
				writeScalarType(prop->count_external);
				out << " ";
				writeScalarType(prop->external_type);
				out << " " << prop->name << "\n";
			} else if (prop->is_list == SectionType::String) {
				out << "property string ";
				out << " " << prop->name << "\n";
			} else {
				out << "property ";
				writeScalarType(prop->external_type);
				out << " " << prop->name << "\n";
			}
		}
	}
	out << "end_header\n";
}
//
void PLYReaderWriter::putElementInternal(void* elem_ptr) {
	PlyElement *elem;
	PlyProperty *prop;
	char* item;
	char *elem_data;
	char **item_ptr;
	int list_count;
	int item_size;
	int int_val;
	int j, k;
	unsigned int uint_val;
	double double_val;
	char **other_ptr;
	elem = plyFile->which_elem;
	elem_data = (char *) elem_ptr;
	other_ptr = (char **) (((char *) elem_ptr) + elem->other_offset);
	/* write out either to an ascii or binary file */
	if (plyFile->file_type == FileFormat::ASCII) {
		/* write an ascii file */
		/* write out each property of the element */
		for (j = 0; j < (int) elem->props.size(); j++) {
			prop = elem->props[j].get();
			if (elem->store_prop[j] == OTHER_PROP)
				elem_data = *other_ptr;
			else
				elem_data = (char *) elem_ptr;
			if (prop->is_list == SectionType::List) { /* list */
				item = elem_data + prop->count_offset;
				getStoredItem((char *) item, prop->count_internal, &int_val,
						&uint_val, &double_val);
				writeAsciiItem(int_val, uint_val, double_val,
						prop->count_external);
				list_count = uint_val;
				item_ptr = (char **) (elem_data + prop->offset);
				item = item_ptr[0];
				item_size =
						ply_type_size[static_cast<int>(prop->internal_type)];
				for (k = 0; k < list_count; k++) {
					getStoredItem((char *) item, prop->internal_type, &int_val,
							&uint_val, &double_val);
					writeAsciiItem(int_val, uint_val, double_val,
							prop->external_type);
					item += item_size;
				}
			} else if (prop->is_list == SectionType::String) { /* string */
				std::string *str;
				item = elem_data + prop->offset;
				str = (std::string*) item;
				std::string ostr = aly::MakeString() << "\"" << *str << "\"";
				out << ostr;
			} else { /* scalar */
				item = elem_data + prop->offset;
				getStoredItem((char *) item, prop->internal_type, &int_val,
						&uint_val, &double_val);
				writeAsciiItem(int_val, uint_val, double_val,
						prop->external_type);
			}
		}
		out << "\n";
	} else {

		/* write a binary file */

		/* write out each property of the element */
		for (j = 0; j < (int) elem->props.size(); j++) {
			prop = elem->props[j].get();
			if (elem->store_prop[j] == OTHER_PROP)
				elem_data = *other_ptr;
			else
				elem_data = (char *) elem_ptr;
			if (prop->is_list == SectionType::List) { /* list */
				item = elem_data + prop->count_offset;
				item_size =
						ply_type_size[static_cast<int>(prop->count_internal)];
				getStoredItem((char *) item, prop->count_internal, &int_val,
						&uint_val, &double_val);
				writeBinaryItem(int_val, uint_val, double_val,
						prop->count_external);
				list_count = uint_val;
				item_ptr = (char **) (elem_data + prop->offset);
				item = item_ptr[0];
				item_size =
						ply_type_size[static_cast<int>(prop->internal_type)];
				for (k = 0; k < list_count; k++) {
					getStoredItem((char *) item, prop->internal_type, &int_val,
							&uint_val, &double_val);
					writeBinaryItem(int_val, uint_val, double_val,
							prop->external_type);
					item += item_size;
				}
			} else if (prop->is_list == SectionType::String) { /* string */
				int len;
				std::string *str;
				item = elem_data + prop->offset;
				str = (std::string *) item;
				/* write the length */
				len = (int)str->length() + 1;
				out << len;
				out << *str;
			} else { /* scalar */
				item = elem_data + prop->offset;
				item_size =
						ply_type_size[static_cast<int>(prop->internal_type)];
				getStoredItem((char *) item, prop->internal_type, &int_val,
						&uint_val, &double_val);
				writeBinaryItem(int_val, uint_val, double_val,
						prop->external_type);
			}
		}

	}
}
/******************************************************************************
 Specify which elements are going to be written.  This should be called
 before a call to the routine ply_put_element().

 Entry:
 plyFile   - file identifier
 elem_name - name of element we're talking about
 ******************************************************************************/

void PLYReaderWriter::putElementSetup(const std::string& elem_name) {
	PlyElement *elem;
	elem = findElement(elem_name);
	if (elem == nullptr) {
		throw std::runtime_error(
				aly::MakeString()
						<< "put_element_setup_ply: can't find element "
						<< elem_name);
	}
	plyFile->which_elem = elem;
}

/*************/
/*  Reading  */
/*************/

/******************************************************************************
 Given a file pointer, get ready to read PLY data from the file.

 Entry:
 fp - the given file pointer

 Exit:
 nelems     - number of elements in object
 elem_names - list of element names
 returns a pointer to a PlyFile, used to refer to this file, or nullptr if error
 ******************************************************************************/

void PLYReaderWriter::openForReading(const std::string& fileName,
		std::vector<std::string>* elem_names) {
	int i, j;
	int nwords;
	std::vector<std::string> words;
	PlyElement *elem;
	std::string orig_line;
	in.open(fileName.c_str(), ios::in | ios::binary);
	if (!in.is_open()) {
		throw std::runtime_error(
				MakeString() << "Could not open " << fileName
						<< " for writing.");
	}
	/* create record for this object */

	plyFile = std::unique_ptr<PlyFile>(new PlyFile());
	/* read and parse the file's header */

	words = getWords(orig_line);
	nwords = (int)words.size();
	if (words.size() == 0 || words[0] != "ply")
		throw std::runtime_error(
				MakeString() << "Could not find 'ply' keyword.");
	while (words.size() > 0) {
		if (words[0] == "format") {
			if (nwords < 3) {
				throw std::runtime_error(
						MakeString() << "Could not recognize format.");
			}
			plyFile->version =(float) atof(words[2].c_str());
			if (words[1] == "ascii") {
				plyFile->file_type = FileFormat::ASCII;
			} else if (words[1] == "binary_big_endian") {
				plyFile->file_type = FileFormat::BINARY_BE;
			} else if (words[1] == "binary_little_endian") {
				plyFile->file_type = FileFormat::BINARY_LE;
			} else {
				throw std::runtime_error(
						MakeString() << "Could not recognize format.");
			}
		} else if (words[0] == "element") {
			addElement(words);
		} else if (words[0] == "property") {
			addProperty(words);
		} else if (words[0] == "comment") {
			addComment(orig_line);
		} else if (words[0] == "obj_info") {
			addObjInfo(orig_line);
		} else if (words[0] == "end_header") {
			break;
		}
		words = getWords(orig_line);
		nwords =(int) words.size();
	}
	/* create tags for each property of each element, to be used */
	/* later to say whether or not to store each property for the user */

	for (i = 0; i < (int) plyFile->elems.size(); i++) {
		elem = plyFile->elems[i].get();
		elem->store_prop.resize(elem->props.size());
		for (j = 0; j < (int) elem->props.size(); j++)
			elem->store_prop[j] = DONT_STORE_PROP;
		elem->other_offset = NO_OTHER_PROPS; /* no "other" props by default */
	}

	/* set return values about the elements */
	elem_names->resize(plyFile->elems.size());
	for (i = 0; i < (int) plyFile->elems.size(); i++)
		(*elem_names)[i] = plyFile->elems[i]->name;

}

/******************************************************************************
 Get information about a particular element.

 Entry:
 plyFile   - file identifier
 elem_name - name of element to get information about

 Exit:
 nelems   - number of elements of this type in the file
 nprops   - number of properties
 returns a list of properties, or nullptr if the file doesn't contain that elem
 ******************************************************************************/

std::vector<std::shared_ptr<PlyProperty>> PLYReaderWriter::getElementDescription(
		const std::string& elem_name, int *nelems, int *nprops) {
	int i;
	PlyElement *elem;
	std::vector<std::shared_ptr<PlyProperty>> prop_list;
	/* find information about the element */
	elem = findElement(elem_name);
	*nelems = elem->num;
	*nprops = (int)elem->props.size();
	if (elem == nullptr)
		return prop_list;
	for (i = 0; i < (int) elem->props.size(); i++) {
		PlyProperty* prop = new PlyProperty();
		*prop = *elem->props[i];
		prop_list.push_back(std::shared_ptr<PlyProperty>(prop));
	}

	return (prop_list);
}

/******************************************************************************
 Specify which properties of an element are to be returned.  This should be
 called before a call to the routine get_element_ply().

 Entry:
 plyFile   - file identifier
 elem_name - which element we're talking about
 nprops    - number of properties
 prop_list - list of properties
 ******************************************************************************/

void PLYReaderWriter::getElementSetup(const std::string& elem_name,
		std::vector<PlyProperty> prop_list) {
	int i;
	PlyElement *elem;
	PlyProperty *prop;
	int index;

	/* find information about the element */
	elem = findElement(elem_name);
	plyFile->which_elem = elem;
	/* deposit the property information into the element's description */
	for (i = 0; i < (int) prop_list.size(); i++) {
		/* look for actual property */
		prop = findProperty(elem, prop_list[i].name, &index);
		if (prop == nullptr) {
			throw std::runtime_error(
					aly::MakeString() << "Warning:  Can't find property "
							<< prop_list[i].name << " in element " << elem_name
							<< "\n");
			continue;
		}
		/* store its description */
		prop->internal_type = prop_list[i].internal_type;
		prop->offset = prop_list[i].offset;
		prop->count_internal = prop_list[i].count_internal;
		prop->count_offset = prop_list[i].count_offset;
		/* specify that the user wants this property */
		elem->store_prop[index] = STORE_PROP;
	}
}

/******************************************************************************
 Specify a property of an element that is to be returned.  This should be
 called (usually multiple times) before a call to the routine ply_get_element().
 This routine should be used in preference to the less flexible old routine
 called ply_get_element_setup().

 Entry:
 plyFile   - file identifier
 elem_name - which element we're talking about
 prop      - property to add to those that will be returned
 ******************************************************************************/

void PLYReaderWriter::getProperty(const std::string& elem_name,
		const PlyProperty *prop) {
	PlyElement *elem;
	PlyProperty *prop_ptr;
	int index;
	/* find information about the element */
	elem = findElement(elem_name);
	plyFile->which_elem = elem;
	/* deposit the property information into the element's description */
	prop_ptr = findProperty(elem, prop->name, &index);
	if (prop_ptr == nullptr) {
		throw std::runtime_error(
				aly::MakeString() << "Warning:  Can't find property "
						<< prop->name << " in element " << elem_name);
		return;
	}
	prop_ptr->internal_type = prop->internal_type;
	prop_ptr->offset = prop->offset;
	prop_ptr->count_internal = prop->count_internal;
	prop_ptr->count_offset = prop->count_offset;

	/* specify that the user wants this property */
	elem->store_prop[index] = STORE_PROP;
}

/******************************************************************************
 Read one element from the file.  This routine assumes that we're reading
 the type of element specified in the last call to the routine
 ply_get_element_setup().

 Entry:
 plyFile  - file identifier
 elem_ptr - pointer to location where the element information should be put
 ******************************************************************************/

void PLYReaderWriter::getElementInternal(void *elem_ptr) {
	if (plyFile->file_type == FileFormat::ASCII)
		asciiGetElement((char *) elem_ptr);
	else
		binaryGetElement((char *) elem_ptr);
}

/******************************************************************************
 Extract the comments from the header information of a PLY file.

 Entry:
 plyFile - file identifier

 Exit:
 num_comments - number of comments returned
 returns a pointer to a list of comments
 ******************************************************************************/

std::vector<std::string> PLYReaderWriter::getComments() {
	return plyFile->comments;
}

/******************************************************************************
 Extract the object information (arbitrary text) from the header information
 of a PLY file.

 Entry:
 plyFile - file identifier

 Exit:
 num_obj_info - number of lines of text information returned
 returns a pointer to a list of object info lines
 ******************************************************************************/

std::vector<std::string> PLYReaderWriter::getObjInfo() {
	return plyFile->obj_info;
}

/******************************************************************************
 Find a property in the list of properties of a given element.

 Entry:
 elem      - pointer to element in which we want to find the property
 prop_name - name of property to find

 Exit:
 index - index to position in list
 returns a pointer to the property, or nullptr if not found
 ******************************************************************************/

PlyProperty* PLYReaderWriter::findProperty(PlyElement *elem,
		const std::string& prop_name, int *index) {
	int i;
	for (i = 0; i < (int) elem->props.size(); i++)
		if (prop_name == elem->props[i]->name) {
			*index = i;
			return elem->props[i].get();
		}

	*index = -1;
	return (nullptr);
}

/******************************************************************************
 Read an element from an ascii file.

 Entry:
 plyFile  - file identifier
 elem_ptr - pointer to element
 ******************************************************************************/

void PLYReaderWriter::asciiGetElement(char *elem_ptr) {
	int j, k;
	PlyElement *elem = nullptr;
	PlyProperty *prop = nullptr;
	std::vector<std::string> words;
	int which_word;
	char *elem_data = nullptr, *item = nullptr;
	int item_size;
	int int_val;
	unsigned int uint_val;
	double double_val;
	int list_count;
	int store_it;
	char **store_array = nullptr;
	std::string orig_line;
	std::vector<char> other_data;
	int other_flag;

	/* the kind of element we're reading currently */
	elem = plyFile->which_elem;

	/* do we need to setup for other_props? */
	if (elem->other_offset != NO_OTHER_PROPS) {
		char **ptr;
		other_flag = 1;
		/* make room for other_props */
		other_data.resize(elem->other_size);
		/* store pointer in user's structure to the other_props */
		ptr = (char **) (elem_ptr + elem->other_offset);
		*ptr = other_data.data();
	} else
		other_flag = 0;

	/* read in the element */

	words = getWords(orig_line);
	if (words.size() == 0) {
		throw std::runtime_error("getElement: unexpected end of file\n");
	}

	which_word = 0;

	for (j = 0; j < (int) elem->props.size(); j++) {
		prop = elem->props[j].get();
		store_it = (elem->store_prop[j] | other_flag);

		/* store either in the user's structure or in other_props */
		if (elem->store_prop[j])
			elem_data = elem_ptr;
		else
			elem_data = other_data.data();

		if (prop->is_list == SectionType::List) { /* a list */
			/* get and store the number of items in the list */
			getAsciiItem(words[which_word++], prop->count_external, &int_val,
					&uint_val, &double_val);
			if (store_it) {
				item = elem_data + prop->count_offset;
				storeItem(item, prop->count_internal, int_val, uint_val,
						double_val);
			}

			/* allocate space for an array of items and store a ptr to the array */
			list_count = int_val;
			item_size = ply_type_size[static_cast<int>(prop->internal_type)];
			store_array = (char **) (elem_data + prop->offset);

			if (list_count == 0) {
				if (store_it)
					*store_array = nullptr;
			} else {
				if (store_it) {
					char* item_ptr = new char[item_size * list_count];
					item = item_ptr;
					*store_array = item_ptr;
				}

				/* read items and store them into the array */
				for (k = 0; k < list_count; k++) {
					getAsciiItem(words[which_word++], prop->external_type,
							&int_val, &uint_val, &double_val);
					if (store_it) {
						storeItem(item, prop->internal_type, int_val, uint_val,
								double_val);
						item += item_size;
					}
				}
			}

		} else if (prop->is_list == SectionType::String) { /* a string */
			if (store_it) {
				std::string str = words[which_word++];
				item = elem_data + prop->offset;
			} else {
				which_word++;
			}
		} else {/* a scalar */
			getAsciiItem(words[which_word++], prop->external_type, &int_val,
					&uint_val, &double_val);
			if (store_it) {
				item = elem_data + prop->offset;
				storeItem(item, prop->internal_type, int_val, uint_val,
						double_val);
			}
		}
	}
}

/******************************************************************************
 Read an element from a binary file.

 Entry:
 plyFile  - file identifier
 elem_ptr - pointer to an element
 ******************************************************************************/

void PLYReaderWriter::binaryGetElement(char *elem_ptr) {
	int j, k;
	PlyElement *elem = nullptr;
	PlyProperty *prop = nullptr;
	char *elem_data = nullptr;
	char *item = nullptr;
	int item_size;
	int int_val;
	unsigned int uint_val;
	double double_val;
	int list_count;
	int store_it;
	char **store_array = nullptr;
	char *other_data = nullptr;
	int other_flag;

	/* the kind of element we're reading currently */
	elem = plyFile->which_elem;

	/* do we need to setup for other_props? */

	if (elem->other_offset != NO_OTHER_PROPS) {
		char **ptr;
		other_flag = 1;
		/* make room for other_props */
		other_data = new char[elem->other_size];
		/* store pointer in user's structure to the other_props */
		ptr = (char **) (elem_ptr + elem->other_offset);
		*ptr = other_data;
	} else
		other_flag = 0;

	/* read in a number of elements */

	for (j = 0; j < (int)elem->props.size(); j++) {

		prop = elem->props[j].get();
		store_it = (elem->store_prop[j] | other_flag);

		/* store either in the user's structure or in other_props */
		if (elem->store_prop[j])
			elem_data = elem_ptr;
		else
			elem_data = other_data;

		if (prop->is_list == SectionType::List) { /* list */

			/* get and store the number of items in the list */
			getBinaryItem(prop->count_external, &int_val, &uint_val,
					&double_val);
			if (store_it) {
				item = elem_data + prop->count_offset;
				storeItem(item, prop->count_internal, int_val, uint_val,
						double_val);
			}

			/* allocate space for an array of items and store a ptr to the array */
			list_count = int_val;
			item_size = ply_type_size[static_cast<int>(prop->internal_type)];
			store_array = (char **) (elem_data + prop->offset);
			if (list_count == 0) {
				if (store_it)
					*store_array = nullptr;
			} else {
				if (store_it) {
					char* item_ptr = new char[item_size * list_count];
					item = item_ptr;
					*store_array = item_ptr;
				}

				/* read items and store them into the array */
				for (k = 0; k < list_count; k++) {
					getBinaryItem(prop->external_type, &int_val, &uint_val,
							&double_val);
					if (store_it) {
						storeItem(item, prop->internal_type, int_val, uint_val,
								double_val);
						item += item_size;
					}
				}
			}

		} else if (prop->is_list == SectionType::String) { /* string */
			int len;
			in.read((char*) &len, sizeof(int));
			std::vector<char> buff(len);
			in.read(buff.data(), buff.size());
			std::string str = std::string(buff.begin(), buff.end());
			if (store_it) {
				item = elem_data + prop->offset;
			}
		} else { /* scalar */
			getBinaryItem(prop->external_type, &int_val, &uint_val,
					&double_val);
			if (store_it) {
				item = elem_data + prop->offset;
				storeItem(item, prop->internal_type, int_val, uint_val,
						double_val);
			}
		}

	}
}

/******************************************************************************
 Write to a file the word that represents a PLY data type.

 Entry:
 fp   - file pointer
 code - code for type
 ******************************************************************************/

void PLYReaderWriter::writeScalarType(const DataType& code) {
	/* make sure this is a valid code */

	if (static_cast<int>(code) <= static_cast<int>(DataType::StartType)
			|| static_cast<int>(code) >= static_cast<int>(DataType::EndType)) {
		throw std::runtime_error(
				aly::MakeString() << "writeScalarType: bad data code = "
						<< code);
	}
	/* write the code to a file */
	out << property_type_names[static_cast<int>(code)];
}

void PLYReaderWriter::writeScalarType(int code) {
	/* make sure this is a valid code */

	if (code <= static_cast<int>(DataType::StartType)
			|| code >= static_cast<int>(DataType::EndType)) {
		throw std::runtime_error(
				aly::MakeString() << "writeScalarType: bad data code = "
						<< code);
	}
	/* write the code to a file */
	out << property_type_names[code];
}
/******************************************************************************
 Get a text line from a file and break it up into words.

 IMPORTANT: The calling routine should call "free" on the returned pointer once
 finished with it.

 Entry:
 fp - file to read from

 Exit:
 nwords    - number of words returned
 orig_line - the original line of characters
 returns a list of words from the line, or nullptr if end-of-file
 ******************************************************************************/

std::vector<std::string> PLYReaderWriter::getWords(std::string& orig_line) {
	std::string line;
	std::vector<std::string> result;
	getline(in, line);
	if (line.size() == 0)
		return result;
	std::stringstream ss;
	std::string comp;
	for (char c : line) {
		if (c == ' ' || c == '\t' || c == '\n') {
			comp = ss.str();
			if (comp.size() > 0) {
				result.push_back(comp);
				ss.str("");
			}
		} else {
			if (c != '\r')
				ss << c;
		}
	}
	comp = ss.str();
	if (comp.size() > 0) {
		result.push_back(comp);
	}
	orig_line = line;
	if (orig_line.size() > 0 && orig_line[orig_line.size() - 1] == '\r') {
		orig_line = orig_line.substr(0, orig_line.size() - 1);
	}
	return result;
}
/******************************************************************************
 Return the value of an item, given a pointer to it and its type.

 Entry:
 item - pointer to item
 type - data type that "item" points to

 Exit:
 returns a double-precision float that contains the value of the item
 ******************************************************************************/

double PLYReaderWriter::getItemValue(char* item, const DataType& type) {
	unsigned char *puchar;
	char *pchar;
	short int *pshort;
	unsigned short int *pushort;
	int *pint;
	unsigned int *puint;
	float *pfloat;
	double *pdouble;
	int int_value;
	unsigned int uint_value;
	double double_value;
	switch (type) {
	case DataType::Int8:
		pchar = (char *) item;
		int_value =(char) *pchar;
		return ((double) int_value);
	case DataType::Uint8:
		puchar = (unsigned char *) item;
		int_value =(unsigned char) *puchar;
		return ((double) int_value);
	case DataType::Int16:
		pshort = (short int *) item;
		int_value =(short int) *pshort;
		return ((double) int_value);
	case DataType::Uint16:
		pushort = (unsigned short int *) item;
		int_value =(unsigned short int) *pushort;
		return ((double) int_value);
	case DataType::Int32:
		pint = (int *) item;
		int_value = (int)*pint;
		return ((double) int_value);
	case DataType::Uint32:
		puint = (unsigned int *) item;
		uint_value = (unsigned int)*puint;
		return ((double) uint_value);
	case DataType::Float32:
		pfloat = (float *) item;
		double_value =(float) *pfloat;
		return (double_value);
	case DataType::Float64:
		pdouble = (double *) item;
		double_value = (double)*pdouble;
		return (double_value);
	default:
		throw std::runtime_error(
				MakeString() << "get_item_value: bad type = " << type);
	}

	return (0.0); /* never actually gets here */
}

/******************************************************************************
 Write out an item to a file as raw binary bytes.

 Entry:
 fp         - file to write to
 int_val    - integer version of item
 uint_val   - unsigned integer version of item
 double_val - double-precision float version of item
 type       - data type to write out
 ******************************************************************************/

void PLYReaderWriter::writeBinaryItem(int int_val, unsigned int uint_val,
		double double_val, const DataType& type) {
	unsigned char uchar_val;
	char char_val;
	unsigned short ushort_val;
	short short_val;
	float float_val;

	switch (type) {
	case DataType::Int8:
		char_val =(char) int_val;
		out.write(&char_val, sizeof(char_val));
		break;
	case DataType::Int16:
		short_val =(short) int_val;
		out.write((char*) &short_val, sizeof(short_val));
		break;
	case DataType::Int32:
		out.write((char*) &int_val, sizeof(int_val));
		break;
	case DataType::Uint8:
		uchar_val =(unsigned char) uint_val;
		out.write((char*) &uchar_val, sizeof(uchar_val));
		break;
	case DataType::Uint16:
		ushort_val = uint_val;
		out.write((char*) &ushort_val, sizeof(ushort_val));
		break;
	case DataType::Uint32:
		out.write((char*) &uint_val, sizeof(uint_val));
		break;
	case DataType::Float32:
		float_val = (float)double_val;
		out.write((char*) &float_val, sizeof(float_val));
		break;
	case DataType::Float64:
		out.write((char*) &double_val, sizeof(double_val));
		break;
	default:
		throw std::runtime_error(
				MakeString() << "writeBinaryItem: bad type = " << type);
	}
}
/******************************************************************************
 Write out an item to a file as ascii characters.

 Entry:
 fp         - file to write to
 int_val    - integer version of item
 uint_val   - unsigned integer version of item
 double_val - double-precision float version of item
 type       - data type to write out
 ******************************************************************************/

void PLYReaderWriter::writeAsciiItem(int int_val, unsigned int uint_val,
		double double_val, const DataType& type) {
	std::string str;
	switch (type) {
	case DataType::Int8:
	case DataType::Int16:
	case DataType::Int32:
		str = aly::MakeString() << int_val << " ";
		out << str;
		break;
	case DataType::Uint8:
	case DataType::Uint16:
	case DataType::Uint32:
		str = aly::MakeString() << uint_val << " ";
		out << str;
		break;
	case DataType::Float32:
	case DataType::Float64:
		str = aly::MakeString() << double_val << " ";
		out << str;
		break;
	default:
		throw std::runtime_error(
				aly::MakeString() << "writeAsciiItem: bad type =\n" << type);
	}
}
/******************************************************************************
 Get the value of an item that is in memory, and place the result
 into an integer, an unsigned integer and a double.

 Entry:
 ptr  - pointer to the item
 type - data type supposedly in the item

 Exit:
 int_val    - integer value
 uint_val   - unsigned integer value
 double_val - double-precision floating point value
 ******************************************************************************/

void PLYReaderWriter::getStoredItem(char *ptr, const DataType& type,
		int *int_val, unsigned int *uint_val, double *double_val) {
	switch (type) {
	case DataType::Int8:
		*int_val = (int)*((char *) ptr);
		*uint_val = (unsigned int)*int_val;
		*double_val = (double)*int_val;
		break;
	case DataType::Uint8:
		*uint_val = (unsigned int)*((unsigned char *) ptr);
		*int_val = (int)*uint_val;
		*double_val =(double) *uint_val;
		break;
	case DataType::Int16:
		*int_val =(int) *((short int *) ptr);
		*uint_val = (unsigned int)*int_val;
		*double_val =(double) *int_val;
		break;
	case DataType::Uint16:
		*uint_val =(unsigned int) *((unsigned short int *) ptr);
		*int_val = (int)*uint_val;
		*double_val = (double)*uint_val;
		break;
	case DataType::Int32:
		*int_val = (int)*((int *) ptr);
		*uint_val = (unsigned int)*int_val;
		*double_val = (double)*int_val;
		break;
	case DataType::Uint32:
		*uint_val =(unsigned int) *((unsigned int *) ptr);
		*int_val =(int) *uint_val;
		*double_val = (double)*uint_val;
		break;
	case DataType::Float32:
		*double_val =(double) *((float *) ptr);
		*int_val = (int)*double_val;
		*uint_val = (unsigned int)*double_val;
		break;
	case DataType::Float64:
		*double_val =(double) *((double *) ptr);
		*int_val = (int)*double_val;
		*uint_val = (unsigned int)*double_val;
		break;
	default:
		throw runtime_error(
				aly::MakeString() << "get_stored_item: bad type = " << type);
	}
}

/******************************************************************************
 Get the value of an item from a binary file, and place the result
 into an integer, an unsigned integer and a double.

 Entry:
 fp   - file to get item from
 type - data type supposedly in the word

 Exit:
 int_val    - integer value
 uint_val   - unsigned integer value
 double_val - double-precision floating point value
 ******************************************************************************/

void PLYReaderWriter::getBinaryItem(const DataType& type, int *int_val,
		unsigned int *uint_val, double *double_val) {
	char c[8];
	char *ptr;

	ptr = (char *) c;
	switch (type) {
	case DataType::Int8:
		in.read(ptr, 1);
		*int_val = *((char *) ptr);
		*uint_val = *int_val;
		*double_val = *int_val;
		break;
	case DataType::Uint8:
		in.read(ptr, 1);
		*uint_val = *((unsigned char *) ptr);
		*int_val = *uint_val;
		*double_val = *uint_val;
		break;
	case DataType::Int16:
		in.read(ptr, 2);
		*int_val = *((short int *) ptr);
		*uint_val = *int_val;
		*double_val = *int_val;
		break;
	case DataType::Uint16:
		in.read(ptr, 2);
		*uint_val = *((unsigned short int *) ptr);
		*int_val = *uint_val;
		*double_val = *uint_val;
		break;
	case DataType::Int32:
		in.read(ptr, 4);
		*int_val = *((int *) ptr);
		*uint_val = *int_val;
		*double_val = *int_val;
		break;
	case DataType::Uint32:
		in.read(ptr, 4);
		*uint_val = *((unsigned int *) ptr);
		*int_val = (int)*uint_val;
		*double_val = (double)*uint_val;
		break;
	case DataType::Float32:
		in.read(ptr, 4);
		*double_val = *((float *) ptr);
		*int_val = (int)*double_val;
		*uint_val = (unsigned int)*double_val;
		break;
	case DataType::Float64:
		in.read(ptr, 8);
		*double_val = *((double *) ptr);
		*int_val = (int)*double_val;
		*uint_val = (unsigned int)*double_val;
		break;
	default:
		throw std::runtime_error(
				MakeString() << "getBinaryItem: bad type = " << type);
	}
}

/******************************************************************************
 Extract the value of an item from an ascii word, and place the result
 into an integer, an unsigned integer and a double.

 Entry:
 word - word to extract value from
 type - data type supposedly in the word

 Exit:
 int_val    - integer value
 uint_val   - unsigned integer value
 double_val - double-precision floating point value
 ******************************************************************************/

void PLYReaderWriter::getAsciiItem(const std::string& word,
		const DataType& type, int *int_val, unsigned int *uint_val,
		double *double_val) {
	switch (type) {
	case DataType::Int8:
	case DataType::Uint8:
	case DataType::Int16:
	case DataType::Uint16:
	case DataType::Int32:
		*int_val = atoi(word.c_str());
		*uint_val = *int_val;
		*double_val = *int_val;
		break;

	case DataType::Uint32:
		*uint_val = strtoul(word.c_str(), (char **) nullptr, 10);
		*int_val = *uint_val;
		*double_val = *uint_val;
		break;

	case DataType::Float32:
	case DataType::Float64:
		*double_val = atof(word.c_str());
		*int_val = (int) *double_val;
		*uint_val = (unsigned int) *double_val;
		break;

	default:
		throw std::runtime_error(
				MakeString() << "getAsciiItem: bad type = " << type);
		exit(-1);
	}
}

/******************************************************************************
 Store a value into a place being pointed to, guided by a data type.

 Entry:
 item       - place to store value
 type       - data type
 int_val    - integer version of value
 uint_val   - unsigned integer version of value
 double_val - double version of value

 Exit:
 item - pointer to stored value
 ******************************************************************************/

void PLYReaderWriter::storeItem(char *item, const DataType& type, int int_val,
		unsigned int uint_val, double double_val) {
	unsigned char *puchar;
	short int *pshort;
	unsigned short int *pushort;
	int *pint;
	unsigned int *puint;
	float *pfloat;
	double *pdouble;

	switch (type) {
	case DataType::Int8:
		*item = (char)int_val;
		break;
	case DataType::Uint8:
		puchar = (unsigned char *) item;
		*puchar = (unsigned char)uint_val;
		break;
	case DataType::Int16:
		pshort = (short *) item;
		*pshort = (short)int_val;
		break;
	case DataType::Uint16:
		pushort = (unsigned short *) item;
		*pushort = (unsigned short)uint_val;
		break;
	case DataType::Int32:
		pint = (int *) item;
		*pint = (int)int_val;
		break;
	case DataType::Uint32:
		puint = (unsigned int *) item;
		*puint = (unsigned int)uint_val;
		break;
	case DataType::Float32:
		pfloat = (float *) item;
		*pfloat = (float)double_val;
		break;
	case DataType::Float64:
		pdouble = (double *) item;
		*pdouble = (double)double_val;
		break;
	default:
		throw std::runtime_error(
				MakeString() << "storeItem: bad type = " << type);
	}
}

/******************************************************************************
 Return a list of the names of the elements in a particular PLY file.

 Entry:
 ply - PLY file whose element name list we want

 Exit:
 num_elems  - the number of element names in the list
 returns the list of names
 ******************************************************************************/

std::vector<std::string> PLYReaderWriter::getElementList() {
	std::vector<std::string> elist(plyFile->elems.size());
	for (int i = 0; i < (int)plyFile->elems.size(); i++)
		elist[i] = plyFile->elems[i]->name;
	return elist;
}

/******************************************************************************
 Append a comment to a PLY file.

 Entry:
 ply     - file to append comment to
 comment - the comment to append
 ******************************************************************************/

void PLYReaderWriter::appendComment(const std::string& comment) {
	plyFile->comments.push_back(comment);
}

/******************************************************************************
 Append object information (arbitrary text) to a PLY file.

 Entry:
 ply      - file to append object info to
 obj_info - the object info to append
 ******************************************************************************/

void PLYReaderWriter::appendObjInfo(const std::string& obj_info) {
	plyFile->obj_info.push_back(obj_info);
}

/******************************************************************************
 Specify the index of the next element to be read in from a PLY file.

 Entry:
 ply - file to read from
 index - index of the element to be read

 Exit:
 elem_count - the number of elements in the file
 returns pointer to the name of this next element
 ******************************************************************************/

std::string PLYReaderWriter::setupElementRead(int index, int *elem_count) {
	PlyElement *elem;

	if (index < 0 || index >(int) plyFile->elems.size()) {
		fprintf(stderr, "Warning:  No element with index %d\n", index);
		return (0);
	}

	elem = plyFile->elems[index].get();

	/* set this to be the current element */
	plyFile->which_elem = elem;

	/* return the number of such elements in the file and the element's name */
	*elem_count = elem->num;
	return (elem->name);
}

/******************************************************************************
 Specify one of several properties of the current element that is to be
 read from a file.  This should be called (usually multiple times) before a
 call to the routine get_element_ply().

 Entry:
 plyFile - file identifier
 prop    - property to add to those that will be returned
 ******************************************************************************/

void PLYReaderWriter::setupProperty(PlyProperty *prop) {
	PlyElement *elem;
	PlyProperty *prop_ptr;
	int index;
	elem = plyFile->which_elem;
	/* deposit the property information into the element's description */
	prop_ptr = findProperty(elem, prop->name, &index);
	if (prop_ptr == nullptr) {
		throw std::runtime_error(
				aly::MakeString() << "Warning:  Can't find property "
						<< prop->name << " in element " << elem->name << "\n");
		return;
	}
	prop_ptr->internal_type = prop->internal_type;
	prop_ptr->offset = prop->offset;
	prop_ptr->count_internal = prop->count_internal;
	prop_ptr->count_offset = prop->count_offset;
	/* specify that the user wants this property */
	elem->store_prop[index] = STORE_PROP;
}

/******************************************************************************
 Describe which element is to be written next and state how many of them will
 be written.

 Entry:
 plyFile   - file identifier
 elem_name - name of element that information is being described
 nelems    - number of elements of this type to be written
 ******************************************************************************/

void PLYReaderWriter::describeElement(const std::string& elem_name) {

	PlyElement *elem;

	/* look for appropriate element */
	elem = findElement(elem_name);
	if (elem == nullptr) {
		throw std::runtime_error(
				MakeString() << "describeElement: can't find element "
						<< elem_name);
	}
	/* now this element is the current element */
	plyFile->which_elem = elem;
}

/******************************************************************************
 Describe a property of an element.

 Entry:
 plyFile   - file identifier
 prop      - the new property
 ******************************************************************************/

void PLYReaderWriter::describeProperty(PlyProperty *prop) {
	PlyElement *elem;
	PlyProperty *elem_prop;
	elem = plyFile->which_elem;
	elem_prop = new PlyProperty();
	elem->props.push_back(std::shared_ptr<PlyProperty>(elem_prop));
	elem->store_prop.push_back(NAMED_PROP);
	*elem_prop = *prop;
}

}
}
