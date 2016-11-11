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

/*
 This is a rewrite of Greg Turk's PLY mesh file reader.
 It doesn't leak memory like the original, and you can sell software that uses it free of charge.
 */
#ifndef ALLOYPLY_H_
#define ALLOYPLY_H_
#include <stdio.h>
#include <string>
#include <memory>
#include <vector>
#include <fstream>
#include <list>
#include <stddef.h>
namespace aly
{
	bool SANITY_CHECK_MESH_IO();
namespace ply
{
enum class FileFormat
{
    ASCII = 1, BINARY_BE = 2, BINARY_LE = 3
};
template<class C, class R> std::basic_ostream<C, R> & operator <<(std::basic_ostream<
                                                                          C, R> & ss,
                                                                  const FileFormat& type)
{
    switch (type)
    {
        case FileFormat::ASCII:
            return ss << "ASCII";
        case FileFormat::BINARY_BE:
            return ss << "Binary Big Endian";
        case FileFormat::BINARY_LE:
            return ss << "Binary Little Endian";
    }
    return ss;
}
enum class DataType
{
    StartType = 0,
    Int8 = 1,
    Int16 = 2,
    Int32 = 3,
    Uint8 = 4,
    Uint16 = 5,
    Uint32 = 6,
    Float32 = 7,
    Float64 = 8,
    EndType = 9
};



const std::string property_type_names[] =
{ "invalid", "int8", "int16", "int32", "uint8", "uint16", "uint32", "float32",
        "float64" };
const std::string old_property_type_names[] =
{ "invalid", "char", "short", "int", "uchar", "ushort", "uint", "float",
        "double" };
const int ply_type_size[] =
{ 0, 1, 2, 4, 1, 2, 4, 4, 8 };
template<class C, class R> std::basic_ostream<C, R> & operator <<(std::basic_ostream<
                                                                          C, R> & ss,
                                                                  const DataType& type)
{
    return ss << property_type_names[static_cast<int>(type)];
}

enum class SectionType
{
    Scalar = 0, List = 1, String = 2
};
template<class C, class R> std::basic_ostream<C, R> & operator <<(std::basic_ostream<
                                                                          C, R> & ss,
                                                                  const SectionType& type)
{
    switch (type)
    {
        case SectionType::Scalar:
            return ss << "Scalar";
        case SectionType::List:
            return ss << "List";
        case SectionType::String:
            return ss << "String";
    }
    return ss;
}
struct plyVertex
{
        float x[3];             // the usual 3-space position of a vertex
        float n[3];
        float vel[3];
        unsigned char red;
        unsigned char green;
        unsigned char blue;
        unsigned char alpha;
        plyVertex()
        {
            x[0] = 0;
            x[1] = 0;
            x[2] = 0;
            n[0] = 0;
            n[1] = 0;
            n[2] = 0;
            vel[0] = 0;
            vel[1] = 0;
            vel[2] = 0;
            red = 0;
            green = 0;
            blue = 0;
            alpha = 0;
        }
};

struct plyFace
{
        unsigned char nverts;    // number of vertex indices in list
        unsigned char nvels;    // number of vertex indices in list
        int *verts;              // vertex index list
        float* velocity;
        plyFace()
        {
            nverts = 0;
            verts = nullptr;
            velocity = nullptr;
            nvels = 3;
        }
};
struct plyFaceTexture
{
        unsigned char nverts;    // number of vertex indices in list
        unsigned char nvels;    // number of vertex indices in list
        int *verts;              // vertex index list
        float* velocity;
        unsigned char uvcount;
        float* uvs;
        plyFaceTexture()
        {
            nverts = 0;
            uvs = nullptr;
            verts = nullptr;
            velocity = nullptr;
            uvcount = 6;
            nvels = 3;
        }
};
struct plyParticle
{
        float x[3];             // the usual 3-space position of a vertex
        float n[3];
        float radius;
        plyParticle()
        {
            x[0] = 0;
            x[1] = 0;
            x[2] = 0;
            n[0] = 0;
            n[1] = 0;
            n[2] = 0;
            radius = 0.0f;
        }
};
struct plyFaceTexutre
{
        unsigned char nverts;    // number of vertex indices in list
        unsigned char nvels;    // number of vertex indices in list

        int *verts;              // vertex index list
        float* velocity;
        unsigned char uvcount;
        float* uvs;
        plyFaceTexutre()
        {
            nverts = 0;
            uvs = nullptr;
            verts = nullptr;
            velocity = nullptr;
            uvcount = 6;
            nvels = 3;
        }
};
struct PlyProperty
{ /* description of a property */
        std::string name; /* property name */
        DataType external_type; /* file's data type */
        DataType internal_type; /* program's data type */
        int offset; /* offset bytes of prop in a struct */
        SectionType is_list; /* 0 = scalar, 1 = list, 2 = char string */
        DataType count_external; /* file's count type */
        DataType count_internal; /* program's count type */
        int count_offset; /* offset byte for list count */
        PlyProperty(const std::string& name = "",
                    const DataType& external_type = DataType::Float32,
                    const DataType& internal_type = DataType::Float32,
                    size_t offset = 0,
                    const SectionType& sectionType = SectionType::Scalar,
                    const DataType& count_external = DataType::StartType,
                    const DataType& count_internal = DataType::StartType,
                    size_t count_offset = 0) :
                name(name), external_type(external_type), internal_type(internal_type), offset((int) offset), is_list(sectionType), count_external(count_external), count_internal(count_internal), count_offset((int)count_offset)
        {

        }
};
struct PlyElement
{ /* description of an element */
        std::string name; /* element name */
        int num; /* number of elements in this object */
        int size; /* size of element (bytes) or -1 if variable */
        std::vector<std::shared_ptr<PlyProperty>> props; /* list of properties in the file */
        std::vector<char> store_prop; /* flags: property wanted by user? */
        int other_offset; /* offset to un-asked-for props, or -1 if none*/
        int other_size; /* size of other_props structure */
};


struct PlyFile
{ /* description of PLY file */
        FileFormat file_type; /* ascii or binary */
        float version; /* version number of file */
        std::vector<std::string> elemNames;
        std::vector<std::shared_ptr<PlyElement>> elems; /* list of elements */
        std::vector<std::string> comments; /* list of comments */
        std::vector<std::string> obj_info; /* list of object info items */
        PlyElement *which_elem; /* element we're currently reading or writing */
};
const PlyProperty MeshVertProps[] =
{ // property information for a vertex
        PlyProperty("x",
                    DataType::Float32,
                    DataType::Float32,
                    offsetof(plyVertex, x)),
                PlyProperty("y", DataType::Float32, DataType::Float32,
                offsetof(plyVertex, x) + sizeof(float)),
                PlyProperty("z", DataType::Float32, DataType::Float32,
                offsetof(plyVertex, x) + sizeof(float) * 2),
                PlyProperty("nx",
                            DataType::Float32,
                            DataType::Float32,
                            offsetof(plyVertex, n)),
                PlyProperty("ny", DataType::Float32, DataType::Float32,
                offsetof(plyVertex, n) + sizeof(float)),
                PlyProperty("nz", DataType::Float32, DataType::Float32,
                offsetof(plyVertex, n) + sizeof(float) * 2),
                PlyProperty("vx",
                            DataType::Float32,
                            DataType::Float32,
                            offsetof(plyVertex, vel)),
                PlyProperty("vy", DataType::Float32, DataType::Float32,
                offsetof(plyVertex, vel) + sizeof(float)),
                PlyProperty("vz", DataType::Float32, DataType::Float32,
                offsetof(plyVertex, vel) + sizeof(float) * 2),
                PlyProperty("red",
                            DataType::Uint8,
                            DataType::Uint8,
                            offsetof(plyVertex, red)),
                PlyProperty("green",
                            DataType::Uint8,
                            DataType::Uint8,
                            offsetof(plyVertex, green)),
                PlyProperty("blue",
                            DataType::Uint8,
                            DataType::Uint8,
                            offsetof(plyVertex, blue)),
                PlyProperty("alpha",
                            DataType::Uint8,
                            DataType::Uint8,
                            offsetof(plyVertex, alpha)) };

const PlyProperty MeshFaceProps[] =
{ // property information for a face
        PlyProperty("vertex_indices",
                    DataType::Int32,
                    DataType::Int32,
                    offsetof(plyFace, verts),
                    SectionType::List,
                    DataType::Uint8,
                    DataType::Uint8,
                    offsetof(plyFace, nverts)), 
	PlyProperty("velocities",
    DataType::Float32,
    DataType::Float32,
    offsetof(plyFace,velocity),
	SectionType::List,
    DataType::Uint8,
    DataType::Uint8,
    offsetof(plyFace, nvels)),
                PlyProperty("vertex_indices",
                            DataType::Int32,
                            DataType::Int32,
                            offsetof(plyFaceTexture, verts),
                            SectionType::List,
                            DataType::Uint8,
                            DataType::Uint8,
                            offsetof(plyFaceTexture, nverts)),
                PlyProperty("texcoord",
                            DataType::Float32,
                            DataType::Float32,
                            offsetof(plyFaceTexture, uvs),
                            SectionType::List,
                            DataType::Uint8,
                            DataType::Uint8,
                            offsetof(plyFaceTexture, uvcount)),
                PlyProperty("velocities",
                            DataType::Float32,
                            DataType::Float32,
                            offsetof(plyFaceTexture, velocity),
                            SectionType::List,
                            DataType::Uint8,
                            DataType::Uint8,
                            offsetof(plyFaceTexture, nvels)) };
class PLYReaderWriter
{
    protected:
        void putElementInternal(void *);
        void getElementInternal(void *);
    public:
        int getNumberOfElements() const
        {
            return (int)plyFile->elems.size();
        }
        std::vector<std::string> getElementNames()
        {
            return plyFile->elemNames;
        }
        void openForWriting(const std::string& fileName,
                            const std::vector<std::string>& elem_names,
                            const FileFormat& file_type);
        void openForReading(const std::string& fileName);
        void elementCount(const std::string&, int);
        void describeElement(const std::string&);
        void describeProperty(const std::string&, const PlyProperty *);
        void appendComment(const std::string&);
        void appendObjInfo(const std::string&);
        void headerComplete();
        void putElementSetup(const std::string&);
        template<class T> void putElement(T * data)
        {
            putElementInternal((void*) data);
        }
        void getProperty(const std::string&, const PlyProperty *);
        template<class T> void getElement(T * data)
        {
            getElementInternal((void*) data);
        }
        PlyElement *findElement(const std::string&);
        PlyProperty *findProperty(PlyElement *, const std::string&, int *);
        std::vector<std::shared_ptr<PlyProperty>> getElementDescription(const std::string& elem_name,
                                                                        int *nelems,
                                                                        int *nprops);
        void describeProperty(PlyProperty *);
        std::vector<std::string> getComments();
        std::vector<std::string> getObjInfo();
        std::vector<std::string> getElementList();
        void setupProperty(PlyProperty *);
        DataType getPropType(const std::string& type_name);
    protected:
        static const int NO_OTHER_PROPS = -1;
        static const int DONT_STORE_PROP = 0;
        static const int STORE_PROP = 1;
        static const int OTHER_PROP = 0;
        static const int NAMED_PROP = 1;
        std::ofstream out;
        std::ifstream in;

        std::unique_ptr<PlyFile> plyFile;
        void openForReading(const std::string& fileName,
                            std::vector<std::string>* elem_names);
        PlyFile* write(const std::vector<std::string>& elem_names,
                       const FileFormat& file_type);
        void getElementSetup(const std::string& elem_name,
                             std::vector<PlyProperty> prop_list);
        void elementLayout(const std::string&,
                           int,
                           int,
                           const std::vector<PlyProperty>& prop_list);
        void addComment(const std::string&);
        void addObjInfo(const std::string&);
        void addElement(const std::vector<std::string>&);
        void addProperty(const std::vector<std::string>& words);
        void writeScalarType(int);
        void writeScalarType(const DataType&);
        std::vector<std::string> getWords(std::string& orig_line);
        void writeBinaryItem(int, unsigned int, double, const DataType& type);
        void writeAsciiItem(int, unsigned int, double, const DataType& type);
        void storeItem(char*, const DataType&, int, unsigned int, double);
        std::string setupElementRead(int index, int *elem_count);
        void getStoredItem(char *,
                           const DataType&,
                           int *,
                           unsigned int *,
                           double *);
        double getItemValue(char*, const DataType&);
        void getAsciiItem(const std::string&,
                          const DataType&,
                          int *,
                          unsigned int *,
                          double *);
        void getBinaryItem(const DataType& type,
                           int *,
                           unsigned int *,
                           double *);
        void asciiGetElement(char*);
        void binaryGetElement(char*);
};
}
}
#endif
