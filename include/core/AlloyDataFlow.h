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
#ifndef ALLOYUALGRAPH_H_
#define ALLOYUALGRAPH_H_
#include "AlloyAny.h"
#include "AlloyUI.h"
#include "AvoidanceRouting.h"
#include "AlloyUndoRedo.h"
#include "AlloyWidget.h"
#include <string>
#include <vector>
#include <memory>
#include <chrono>
namespace aly {
namespace dataflow {
class Node;
class Data;
class Compute;
class View;
class DataFlow;
class Connection;
struct BoxForce;
class ForceSimulator;
struct ForceItem;
struct SpringItem;

enum class NodeType {
	Unknown = 0, Data = 1, View = 2, Compute = 3, Group = 4, Source = 5, Destination = 6
};

enum class NodeShape {
	Hidden = 0, Circle = 1, CircleGroup = 2, Triangle = 3, Square = 4, Hexagon = 5
};
enum class NodeAction {
	Focus, Select, DoubleClick, Add, Delete, Modify, Copy, Paste
};
enum class ConnectionAction {
	Focus, Select, Add, Delete
};
template<class C, class R> std::basic_ostream<C, R> & operator <<(std::basic_ostream<C, R> & ss, const NodeAction& type) {
	switch (type) {
	case NodeAction::Focus:
		return ss << "Focus";
	case NodeAction::Select:
		return ss << "Select";
	case NodeAction::DoubleClick:
		return ss << "Double Click";
	case NodeAction::Add:
		return ss << "Add";
	case NodeAction::Delete:
		return ss << "Delete";
	case NodeAction::Modify:
		return ss << "Modify";
	case NodeAction::Copy:
		return ss << "Copy";
	case NodeAction::Paste:
		return ss << "Paste";
	}
	return ss;
}
template<class C, class R> std::basic_ostream<C, R> & operator <<(std::basic_ostream<C, R> & ss, const ConnectionAction& type) {
	switch (type) {
	case ConnectionAction::Focus:
		return ss << "Focus";
	case ConnectionAction::Select:
		return ss << "Select";
	case ConnectionAction::Add:
		return ss << "Add";
	case ConnectionAction::Delete:
		return ss << "Delete";
	}
	return ss;
}
enum class PortType {
	Unknown = 0, Input = 1, Output = 2, Child = 3, Parent = 4
};
template<class C, class R> std::basic_ostream<C, R> & operator <<(std::basic_ostream<C, R> & ss, const PortType& type) {
	switch (type) {
	case PortType::Unknown:
		return ss << "Unknown";
	case PortType::Input:
		return ss << "Input";
	case PortType::Output:
		return ss << "Output";
	case PortType::Child:
		return ss << "Child";
	case PortType::Parent:
		return ss << "Parent";
	}
	return ss;
}
template<class C, class R> std::basic_ostream<C, R> & operator <<(std::basic_ostream<C, R> & ss, const NodeType& type) {
	switch (type) {
	case NodeType::Unknown:
		return ss << "Unknown";
	case NodeType::Data:
		return ss << "Data";
	case NodeType::View:
		return ss << "View";
	case NodeType::Compute:
		return ss << "Compute";
	case NodeType::Group:
		return ss << "Group";
	case NodeType::Source:
		return ss << "Source";
	case NodeType::Destination:
		return ss << "Destination";
	}
	return ss;
}

struct Packet: public AnyInterface {
protected:
	virtual void setValueImpl(Any const & value) = 0;
	virtual Any getValueImpl() const = 0;
};
template<class T> class PacketImpl: public Packet {
protected:
	T value;
	std::string name;
	virtual void setValueImpl(Any const & val) override {
		value = AnyCast<T>(val);
	}
	virtual Any getValueImpl() const override {
		return value;
	}
public:
	PacketImpl(const std::string& name, const T& value) :
			value(value), name(name) {
	}
	PacketImpl(const std::string& name = "") :
			name(name) {
	}
	std::string getName() const {
		return name;
	}
	void setName(const std::string& name) {
		this->name = name;
	}
};
class ConnectionBundle: public std::vector<std::shared_ptr<Connection>> {
public:
	ConnectionBundle() :
			std::vector<std::shared_ptr<Connection>>() {
	}

};
class Port: public Region {
protected:
	Node* parent;
	std::string label;
	std::string id;
	std::shared_ptr<Port> proxyIn;
	std::shared_ptr<Port> proxyOut;
	ConnectionBundle connections;
	Any contents;
	virtual void setup();
public:
	friend class Connection;
	friend class Node;
	DataFlow* getGraph() const;
	inline Node* getNode() const {
		return parent;
	}
	template<class T> inline void setContents(T&& val) {
		contents = val;
	}
	std::string getId() const {
		return id;
	}
	void setId(const std::string& id) {
		this->id = id;
	}
	std::string getReferenceId() const;
	void copyContentsFrom(const Port* port) {
		if (port->hasContents()) {
			this->contents = Any(port->contents);
		}
	}
	void copyContentsFrom(const std::shared_ptr<Port>& port) {
		if (port->hasContents()) {
			this->contents = Any(port->contents);
		}
	}
	void clearContents() {
		contents.clear();
	}
	virtual std::shared_ptr<Port> clone() const = 0;
	bool hasContents() const {
		return !contents.isEmpty();
	}
	template<class T> T getContents() const {
		return AnyCast<T>(contents);
	}
	std::shared_ptr<Port> getProxyOut() const {
		return proxyOut;
	}
	std::shared_ptr<Port> getProxyIn() const {
		return proxyIn;
	}
	bool hasProxyOut() const {
		return (proxyOut.get() != nullptr);
	}
	bool hasProxyIn() const {
		return (proxyIn.get() != nullptr);
	}
	void setProxyOut(const std::shared_ptr<Port>& proxy) {
		proxyOut = proxy;
	}
	void setProxyOut() {
		proxyOut.reset();
	}
	void setProxyIn(const std::shared_ptr<Port>& proxy) {
		proxyIn = proxy;
	}
	void setProxyIn() {
		proxyIn.reset();
	}
	ConnectionBundle& getConnections() {
		return connections;
	}
	const ConnectionBundle& getConnections() const {
		return connections;
	}
	bool disconnect(Connection* connection);

	bool disconnect(const std::shared_ptr<Connection>& connection) {
		return disconnect(connection.get());
	}
	bool isExternalized() const {
		return (proxyIn != nullptr);
	}
	bool isConnected() const {
		return (connections.size() > 0);
	}
	bool connect(const std::shared_ptr<Connection>& connection) {
		//Do not add connections twice!
		for (const std::shared_ptr<Connection> con : connections) {
			if (con.get() == connection.get()) {
				return false;
			}
		}
		connections.push_back(connection);
		return true;
	}

	void setParent(Node* parent) {
		this->parent = parent;
	}
	float2 getLocation() const;
	std::shared_ptr<Port> getReference();
	Port(const std::string& name, const std::string& label) :
			Region(name), parent(nullptr), label(label), proxyIn(nullptr), proxyOut(nullptr) {
		setup();
	}
	Port(const std::string& name) :
			Region(name), parent(nullptr), label(name), proxyIn(nullptr), proxyOut(nullptr) {
		setup();
	}
	virtual PortType getType() const {
		return PortType::Unknown;
	}
	std::string getLabel() const {
		return label;
	}
	void setLabel(const std::string& label) {
		this->label = label;
	}
	virtual void draw(AlloyContext* context) override;
	virtual void setValue(const std::shared_ptr<Packet>& packet) {
	}
	;
	virtual ~Port() {
	}
};
class MultiPort {
protected:
	static const int FrontIndex;
	static const int BackIndex;
public:
	virtual void insertValue(const std::shared_ptr<Packet>& packet, int index)=0;
	virtual void setValue(const std::shared_ptr<Packet>& packet, int index)=0;
	inline void insertAtFront(const std::shared_ptr<Packet>& packet) {
		insertValue(packet, FrontIndex);
	}
	inline void insertAtBack(const std::shared_ptr<Packet>& packet) {
		insertValue(packet, BackIndex);
	}
	virtual ~MultiPort() {
	}
};
class InputPort: public Port {
protected:
	std::shared_ptr<Packet> value;
	virtual void setup() override;
public:
	friend class Node;
	static const pixel2 DIMENSIONS;
	InputPort(const std::string& name) :
			Port(name) {
		setup();
	}
	InputPort(const std::string& name, const std::string& label) :
			Port(name, label) {
		setup();
	}
	virtual std::shared_ptr<Port> clone() const override;

	virtual void pack(const pixel2& pos, const pixel2& dims, const double2& dpmm, double pixelRatio, bool clamp = false) override;
	virtual PortType getType() const override {
		return PortType::Input;
	}
	virtual void setValue(const std::shared_ptr<Packet>& packet) override {
		this->value = packet;
	}

	virtual ~InputPort() {
	}
	virtual void draw(AlloyContext* context) override;
};

class OutputPort: public Port {
protected:
	std::shared_ptr<Packet> value;
	virtual void setup() override;
public:
	friend class Node;
	static const pixel2 DIMENSIONS;
	OutputPort(const std::string& name) :
			Port(name) {
		setup();
	}
	OutputPort(const std::string& name, const std::string& label) :
			Port(name, label) {
		setup();
	}
	virtual std::shared_ptr<Port> clone() const override;

	virtual void pack(const pixel2& pos, const pixel2& dims, const double2& dpmm, double pixelRatio, bool clamp = false) override;
	virtual PortType getType() const override {
		return PortType::Output;
	}
	virtual void setValue(const std::shared_ptr<Packet>& packet) override {
		this->value = packet;
	}
	virtual ~OutputPort() {
	}
	virtual void draw(AlloyContext* context) override;
};

class ParentPort: public Port {
protected:
	std::shared_ptr<Packet> value;
	virtual void setup() override;
public:
	static const pixel2 DIMENSIONS;
	ParentPort(const std::string& name) :
			Port(name) {
		setup();
	}
	ParentPort(const std::string& name, const std::string& label) :
			Port(name, label) {
		setup();
	}
	virtual std::shared_ptr<Port> clone() const override;

	virtual PortType getType() const override {
		return PortType::Parent;
	}
	virtual void setValue(const std::shared_ptr<Packet>& packet) override {
		this->value = packet;
	}
	virtual void pack(const pixel2& pos, const pixel2& dims, const double2& dpmm, double pixelRatio, bool clamp = false) override;
	virtual ~ParentPort() {
	}
	virtual void draw(AlloyContext* context) override;
};
class Connection {
protected:
	std::shared_ptr<SpringItem> springItem;
	Any contents;
public:
	bool selected = false;
	std::shared_ptr<Port> source;
	std::shared_ptr<Port> destination;
	std::vector<float2> path;
	void setSelected(bool b) {
		selected = b;
	}
	bool isSelected() const {
		return selected;
	}

	bool remove();
	std::shared_ptr<SpringItem>& getSpringItem();
	Connection(const std::shared_ptr<Port>& source, const std::shared_ptr<Port>& destination) :
			source(source), destination(destination) {

	}
	template<class T> inline void setContents(T&& val) {
		contents = val;
	}

	void clearContents() {
		contents.clear();
	}
	bool hasContents() const {
		return !contents.isEmpty();
	}
	void copyContentsFrom(const Connection* connection) {
		if (connection->hasContents()) {
			this->contents = Any(connection->contents);
		}
	}
	void copyContentsFrom(const std::shared_ptr<Connection>& connection) {
		if (connection->hasContents()) {
			this->contents = Any(connection->contents);
		}
	}
	template<class T> T getContents() const {
		return AnyCast<T>(contents);
	}
	void update();
	~Connection();
	float distance(const float2& pt);
	void draw(AlloyContext* context, DataFlow* flow);
};

class PacketBundle: public std::vector<std::shared_ptr<Packet>> {
public:
	PacketBundle() :
			std::vector<std::shared_ptr<Packet>>() {
	}

};
class InputMultiPort: public Port, MultiPort {
protected:
	PacketBundle value;
public:
	virtual PortType getType() const override {
		return PortType::Input;
	}
	virtual void insertValue(const std::shared_ptr<Packet>& packet, int index) override;
	virtual void setValue(const std::shared_ptr<Packet>& packet, int index) override;
	virtual void setValue(const std::shared_ptr<Packet>& packet) override;
	virtual ~InputMultiPort() {
	}

};
class OutputMultiPort: public Port, MultiPort {
protected:
	PacketBundle value;
public:
	virtual PortType getType() const override {
		return PortType::Output;
	}
	virtual void insertValue(const std::shared_ptr<Packet>& packet, int index) override;
	virtual void setValue(const std::shared_ptr<Packet>& packet, int index) override;
	virtual void setValue(const std::shared_ptr<Packet>& packet) override;
	virtual ~OutputMultiPort() {
	}
};

class ChildPort: public Port {
protected:
	std::shared_ptr<Packet> value;
	virtual void setup() override;
public:

	static const pixel2 DIMENSIONS;
	ChildPort(const std::string& name) :
			Port(name) {
		setup();
	}
	virtual std::shared_ptr<Port> clone() const override;

	ChildPort(const std::string& name, const std::string& label) :
			Port(name, label) {
		setup();
	}
	virtual PortType getType() const override {
		return PortType::Child;
	}
	virtual void setValue(const std::shared_ptr<Packet>& packet) override {
		this->value = packet;
	}
	virtual void pack(const pixel2& pos, const pixel2& dims, const double2& dpmm, double pixelRatio, bool clamp = false) override;
	virtual ~ChildPort() {
	}
	virtual void draw(AlloyContext* context) override;
};
class Predicate {
protected:
	const std::string name;
public:
	Predicate(const std::string& name) :
			name(name) {
	}
	std::shared_ptr<Predicate> clone() const;
	std::string getName() const;
};
class Relationship {
protected:
	std::shared_ptr<SpringItem> springItem;
public:
	std::shared_ptr<Node> subject;
	std::shared_ptr<Node> object;
	std::shared_ptr<Predicate> predicate;
	std::shared_ptr<SpringItem>& getSpringItem();
	Relationship(const std::shared_ptr<Node>& subject, const std::shared_ptr<Predicate>& predicate, const std::shared_ptr<Node>& object) :
			subject(subject), object(object), predicate(predicate) {
	}
	void update();
	void draw(AlloyContext* context, float scale);
	void drawText(AlloyContext* context, float scale);

};

class NodeIcon: public Region {
protected:
	NodeShape shape;
public:
	float borderScale = 1.0f;
	bool selected = false;
	NodeIcon(const std::string& name, const AUnit2D& pos, const AUnit2D& dims) :
			Region(name, pos, dims), shape(NodeShape::Circle) {
	}

	void setShape(const NodeShape& s) {
		shape = s;
	}
	NodeShape getShape() const {
		return shape;
	}
	virtual void draw(AlloyContext* context) override;
};
class Node: public Composite {
protected:
	std::string label;
	std::string id;
	float fontSize;
	float textWidth;
	pixel2 centerOffset;
	DataFlow* parentFlow;
	std::vector<std::shared_ptr<InputPort>> inputPorts;
	std::vector<std::shared_ptr<OutputPort>> outputPorts;
	std::shared_ptr<InputPort> inputPort;
	std::shared_ptr<OutputPort> outputPort;
	std::shared_ptr<ParentPort> parentPort;
	std::shared_ptr<ChildPort> childPort;
	CompositePtr inputPortComposite;
	CompositePtr outputPortComposite;
	ModifiableLabelPtr labelRegion;
	std::shared_ptr<ForceItem> forceItem;
	Any contents;
	virtual void setup();
	virtual void pack(const pixel2& pos, const pixel2& dims, const double2& dpmm, double pixelRatio, bool clamp = false) override;
	std::function<void(float scale)> resizeFunc;
public:
	static std::string MakeID(int len=8);
	bool isSelected() const {
		return nodeIcon->selected;
	}
	std::string getId() const {
		return id;
	}
	void setId(const std::string& id) {
		this->id = id;
	}
	void setSelected(bool b) {
		nodeIcon->selected = b;
	}
	void copyContentsFrom(const Node* node) {
		if (node->hasContents()) {
			this->contents = Any(node->contents);
		}
	}
	void copyContentsFrom(const std::shared_ptr<Node>& node) {
		if (node->hasContents()) {
			this->contents = Any(node->contents);
		}
	}
	virtual std::shared_ptr<Node> clone() const = 0;
	float2 getLocation() const;
	void setLocation(const float2& pt);
	std::shared_ptr<NodeIcon> nodeIcon;
	friend class DataFlow;
	friend class Port;
	static const pixel2 DIMENSIONS;
	std::shared_ptr<ForceSimulator> getForceSimulator();
	void setParentFlow(DataFlow* parent) {
		this->parentFlow = parent;
	}
	bool hasExternalInput() const;
	bool hasExternalOutput() const;
	bool hasExternalPorts() const {
		return (hasExternalInput() || hasExternalOutput());
	}

	const std::shared_ptr<InputPort>& getInputPort(size_t i) const {
		if(i>=inputPorts.size()){
			throw std::runtime_error(MakeString()<<"Module "<<getName()<<" input port index out of bounds "<<i<<" "<<inputPorts.size());
		}
		return inputPorts[i];
	}
	const std::shared_ptr<OutputPort>& getOutputPort(size_t i) const {
		if(i>=outputPorts.size()){
			throw std::runtime_error(MakeString()<<"Module "<<getName()<<" output port index out of bounds "<<i<<" "<<outputPorts.size());
		}
		return outputPorts[i];
	}
	const std::vector<std::shared_ptr<InputPort>>& getInputPorts() const {
		return inputPorts;
	}
	const std::vector<std::shared_ptr<OutputPort>>& getOutputPorts() const {
		return outputPorts;
	}
	size_t getInputSize() const {
		return inputPorts.size();
	}
	size_t getOutputSize() const {
		return outputPorts.size();
	}
	std::vector<std::shared_ptr<InputPort>>& getInputPorts() {
		return inputPorts;
	}
	std::vector<std::shared_ptr<OutputPort>>& getOutputPorts() {
		return outputPorts;
	}
	const std::shared_ptr<InputPort>& getInputPort() const {
		return inputPort;
	}
	const std::shared_ptr<OutputPort>& getOutputPort() const {
		return outputPort;
	}
	const std::shared_ptr<ParentPort>& getParentPort() const {
		return parentPort;
	}
	const std::shared_ptr<ChildPort>& getChildPort() const {
		return childPort;
	}
	template<class T> inline void setContents(T&& val) {
		contents = val;
	}
	void clearContents() {
		contents.clear();
	}

	bool hasContents() const {
		return !contents.isEmpty();
	}
	template<class T> T getContents() const {
		return AnyCast<T>(contents);
	}
	box2px getObstacleBounds() const;
	DataFlow* getGraph() const {
		return parentFlow;
	}
	DataFlow* getParentFlow() const {
		return parentFlow;
	}
	virtual box2px getBounds(bool includeBounds = true) const override;
	virtual box2px getCursorBounds(bool includeOffset = true) const override;
	virtual pixel2 getDrawOffset() const override;
	virtual NodeType getType() const {
		return NodeType::Unknown;
	}
	bool isMouseOver() const;
	pixel2 getCenter() const {
		return getBoundsPosition() + centerOffset;
	}
	pixel2 getForceOffset() const;
	pixel2 getCenterOffset() const {
		return centerOffset;
	}
	std::shared_ptr<ForceItem>& getForceItem();
	float getRadius() const {
		return nodeIcon->getBounds().dimensions.y * 0.5f;
	}
	Node(const std::string& name, const pixel2& pt);
	Node(const std::string& name, const std::string& label, const pixel2& pt);
	void add(const std::shared_ptr<Region>& region) {
		Composite::add(region);
	}
	void add(const std::shared_ptr<Node>& node) {
		Composite::add(node);
	}
	void add(const std::shared_ptr<InputPort>& port) {
		inputPortComposite->add(port);
		inputPorts.push_back(port);
		if (parent == nullptr) {
			port->parent = this;
		} else
			throw std::runtime_error("Could not add port because it already has parent.");
	}
	void add(const std::shared_ptr<OutputPort>& port) {
		outputPortComposite->add(port);
		outputPorts.push_back(port);
		if (parent == nullptr) {
			port->parent = this;
		} else
			throw std::runtime_error("Could not add port because it already has parent.");
	}
	std::string getLabel() const {
		return label;
	}
	void setLabel(const std::string& label, bool notify = true);
	virtual void draw(AlloyContext* context) override;
};
class Data: public Node {
protected:
	virtual void setup() override;
public:

	static const Color COLOR;
	virtual std::shared_ptr<Node> clone() const override;
	virtual NodeType getType() const override {
		return NodeType::Data;
	}
	Data(const std::string& name, const pixel2& pt) :
			Node(name, pt) {
		setup();
	}
	Data(const std::string& name, const std::string& label, const pixel2& pt) :
			Node(name, label, pt) {
		setup();
	}
	virtual void draw(AlloyContext* context) override;
};
class Group: public Node {
protected:
	virtual void setup() override;
public:
	std::vector<std::shared_ptr<Node>> nodes;
	std::vector<std::shared_ptr<Connection>> connections;
	std::vector<std::shared_ptr<Relationship>> relationships;
	static const Color COLOR;
	virtual std::shared_ptr<Node> clone(bool selectedOnly) const;
	virtual std::shared_ptr<Node> clone() const override {
		return clone(false);
	}
	std::shared_ptr<Connection> getConnection(const std::string& srcId,const std::string& destId) const;
	virtual NodeType getType() const override {
		return NodeType::Group;
	}
	std::string toString(std::string indent = "") const;
	Group(const std::string& name, const pixel2& pt) :
			Node(name, pt) {
		setup();
	}
	Group(const std::string& name, const std::string& label, const pixel2& pt) :
			Node(name, label, pt) {
		setup();
	}
	std::vector<std::shared_ptr<Node>>& getNodes() {
		return nodes;
	}
	std::vector<std::shared_ptr<Node>> getAllChildrenNodes() const;
	const std::vector<std::shared_ptr<Node>>& getNodes() const {
		return nodes;
	}
	std::vector<std::shared_ptr<Relationship>>& getRelationships() {
		return relationships;
	}
	const std::vector<std::shared_ptr<Relationship>>& getRelationships() const {
		return relationships;
	}
	std::vector<std::shared_ptr<Connection>>& getConnections() {
		return connections;
	}
	void add(const std::shared_ptr<Connection>& con) {
		connections.push_back(con);
	}
	void add(const std::shared_ptr<Relationship>& con) {
		relationships.push_back(con);
	}
	void add(const std::shared_ptr<Node>& node) {
		nodes.push_back(node);
	}
	void add(const std::shared_ptr<InputPort>& port) {
		Node::add(port);
	}
	void add(const std::shared_ptr<OutputPort>& port) {
		Node::add(port);
	}
	std::shared_ptr<Connection> getConnection(Connection* con);
	Group* getGroup(Connection* con);
	bool remove(Connection* con);
	bool remove(Relationship* relation);
	const std::vector<std::shared_ptr<Connection>>& getConnections() const {
		return connections;
	}
	virtual void draw(AlloyContext* context) override;
};
class View: public Node {
protected:
	virtual void setup() override;
public:
	static const Color COLOR;
	virtual std::shared_ptr<Node> clone() const override;

	virtual NodeType getType() const override {
		return NodeType::View;
	}
	View(const std::string& name, const pixel2& pt) :
			Node(name, pt) {
		setup();
	}
	View(const std::string& name, const std::string& label, const pixel2& pt) :
			Node(name, label, pt) {
		setup();
	}
	virtual void draw(AlloyContext* context) override;
};
class Compute: public Node {
protected:
	virtual void setup() override;
public:
	static const Color COLOR;
	virtual std::shared_ptr<Node> clone() const override;

	virtual NodeType getType() const override {
		return NodeType::Compute;
	}
	Compute(const std::string& name, const pixel2& pt) :
			Node(name, pt) {
		setup();
	}
	Compute(const std::string& name, const std::string& label, const pixel2& pt) :
			Node(name, label, pt) {
		setup();
	}
	virtual void draw(AlloyContext* context) override;
};
class Source: public Node {
protected:
	virtual void setup() override;

public:

	static const Color COLOR;
	virtual std::shared_ptr<Node> clone() const override;

	virtual NodeType getType() const override {
		return NodeType::Source;
	}
	Source(const std::string& name, const pixel2& pt) :
			Node(name, pt) {
		setup();
	}
	Source(const std::string& name, const std::string& label, const pixel2& pt) :
			Node(name, label, pt) {
		setup();
	}
	virtual void draw(AlloyContext* context) override;
};

class Destination: public Node {
protected:
	virtual void setup() override;

public:

	static const Color COLOR;
	virtual std::shared_ptr<Node> clone() const override;

	virtual NodeType getType() const override {
		return NodeType::Destination;
	}
	Destination(const std::string& name, const pixel2& pt) :
			Node(name, pt) {
		setup();
	}
	Destination(const std::string& name, const std::string& label, const pixel2& pt) :
			Node(name, label, pt) {
		setup();
	}

	virtual void draw(AlloyContext* context) override;
};
class ActionDataFlow;
class DataFlow: public Composite {
protected:
	aly::MessageDialogPtr errorMessage;
	std::shared_ptr<Group> data;
	std::shared_ptr<BoxForce> boxForce;
	std::list<std::pair<Node*, pixel2>> dragList;
	box2px dragBox;
	box2px graphBounds;
	Node* mouseOverNode;
	Node* mouseDragNode;
	Node* mouseSelectedNode;
	Port* connectingPort;
	Port* currentPort;
	std::mutex routingLock;
	AvoidanceRouting router;
	ActionHistory history;
	std::shared_ptr<ForceSimulator> forceSim;
	pixel2 currentDrawOffset;
	pixel2 cursorDownLocation;
	Connection* selectedConnection = nullptr;
	bool draggingGraph = false;
	bool dragAction = false;
	bool draggingNode = false;
	bool mouseDownInRegion = false;
	bool enabled = true;
	float scale = 1.0f;
	static std::shared_ptr<Group> clipboard;
	void setup();
	bool updateSimulation(uint64_t iter);
	void addConnectionInternal(const std::shared_ptr<Connection>& connection);
	void addRelationshipInternal(const std::shared_ptr<Relationship>& connection);
	void addNodeInternal(const std::shared_ptr<Node>& node);
	Connection* closestConnection(const float2& pt, float tolernace);
public:
	friend class Node;
	friend class ActionDataFlow;
	void zoom(float s);
	void addNode(const std::shared_ptr<Node>& node);
	void removeNode(const std::shared_ptr<Node>& node);
	float getScale() const {
		return scale;
	}
	std::shared_ptr<Group> getClipboard() const {
		return clipboard;
	}
	bool copySelected();
	bool paste(pixel2 position);
	bool cutSelected();
	std::string toString(std::string indent = "") const;
	void setScale(float f, pixel2 pos);
	bool deleteSelected(bool externalCheck = false);
	void addNodes(const std::vector<std::shared_ptr<Node>>& node, const std::vector<std::shared_ptr<Connection>>& connections);
	void removeNodes(const std::vector<std::shared_ptr<Node>>& nodes, const std::vector<std::shared_ptr<Connection>>& connections);

	bool groupSelected();
	bool ungroupSelected();
	std::vector<std::shared_ptr<Node>> ungroupNodes(const std::vector<std::shared_ptr<Node>>& nodes);
	std::shared_ptr<Group> groupNodes(const std::vector<std::shared_ptr<Node>>& nodes);

	void setSimulatorEnabled(bool b);
	bool isSimulatorEnabled() const {
		return enabled;
	}
	std::function<void(Node* node, const NodeAction& action)> onNodeAction;
	std::function<void(Connection* node, const ConnectionAction& action)> onConnectionAction;
	std::function<bool(Port* source, Port* destination)> onValidateConnection;
	std::shared_ptr<ForceSimulator> getForceSimulator() {
		return forceSim;
	}
	std::shared_ptr<Group> getGroup() const {
		return data;
	}
	void start();
	void stop();
	virtual void pack(const pixel2& pos, const pixel2& dims, const double2& dpmm, double pixelRatio, bool clamp = false) override;
	bool isConnecting() const {
		return (connectingPort != nullptr);
	}
	Port* getConnectingPort() const {
		return connectingPort;
	}
	Connection* getSelectedConnection() {
		return selectedConnection;
	}
	void setGroup(const std::shared_ptr<Group>& g);
	void setSelected(Connection* item) {
		selectedConnection = item;
	}
	virtual void setVisible(bool b) {
		if (!b)
			stop();
		Composite::setVisible(b);
	}
	bool intersects(const lineseg2f& ln);
	void startConnection(Port* port);
	void setCurrentPort(Port* currentPort);
	virtual void draw(AlloyContext* context) override;
	virtual bool onEventHandler(AlloyContext* context, const InputEvent& event) override;
	DataFlow(const std::string& name, const AUnit2D& pos, const AUnit2D& dims) :
			Composite(name, pos, dims), mouseOverNode(nullptr), connectingPort(nullptr), currentPort(nullptr), history(name) {
		setup();
	}
	virtual ~DataFlow();
	bool isMouseOverNode(const Node* node) const {
		return (node != nullptr && node == mouseOverNode);
	}
	bool undo();
	bool redo();
	void add(const std::shared_ptr<Region>& region);
	void addRegion(const std::shared_ptr<Node>& region);
	void addRegion(const std::shared_ptr<Connection>& region);
	void add(const std::shared_ptr<Relationship>& relationship);
	void add(const std::shared_ptr<Source>& node);
	void add(const std::shared_ptr<Destination>& node);
	void add(const std::shared_ptr<Data>& node);
	void add(const std::shared_ptr<View>& node);
	void add(const std::shared_ptr<Compute>& node);
	void add(const std::shared_ptr<Group>& node);
	void add(const std::shared_ptr<Connection>& connection);
	void move(const std::shared_ptr<Node>& node, pixel2 position);
	virtual void clear() override;
	bool remove(Connection* connection);
	bool remove(Relationship* relation);
	std::vector<std::shared_ptr<Connection>>& getConnections();
	std::shared_ptr<Connection> getConnection(Connection* con);
	Group* getGroup(Connection* con);
	std::vector<std::shared_ptr<Relationship>>& getRelationships();
	std::vector<std::shared_ptr<Node>>& getNodes();
	const std::vector<std::shared_ptr<Connection>>& getConnections() const;
	const std::vector<std::shared_ptr<Relationship>>& getRelationships() const;
	const std::vector<std::shared_ptr<Node>>& getNodes() const;
};
class ActionDataFlow: public UndoableAction {
protected:
	DataFlow* graph;
public:
	ActionDataFlow(const std::string& name, DataFlow* graph);
};
class ActionAddNode: public ActionDataFlow {
public:
	ActionAddNode(DataFlow* graph, const std::shared_ptr<Node>& node);
};
class ActionAddConnection: public ActionDataFlow {
public:
	ActionAddConnection(DataFlow* graph, const std::shared_ptr<Connection>& con);
};
class ActionAddRelationship: public ActionDataFlow {
public:
	ActionAddRelationship(DataFlow* graph, const std::shared_ptr<Relationship>& con);
};
class ActionRemoveNodes: public ActionDataFlow {
public:
	ActionRemoveNodes(DataFlow* graph, const std::vector<std::shared_ptr<Node>>& nodes, const std::vector<std::shared_ptr<Connection>>& connections);
};
class ActionMoveNode: public ActionDataFlow {
public:
	ActionMoveNode(DataFlow* graph, const std::shared_ptr<Node>& node, pixel2 position);
};
class ActionScaleGraph: public ActionDataFlow {
public:
	ActionScaleGraph(DataFlow* graph, float scale, pixel2 position);
};
class ActionMoveGraph: public ActionDataFlow {
public:
	ActionMoveGraph(DataFlow* graph, pixel2 position);
};
class ActionModifyNode: public ActionDataFlow {
public:
	ActionModifyNode(DataFlow* graph, const std::shared_ptr<Node>& node, const std::string& name);
};
class ActionSelectNodes: public ActionDataFlow {
public:
	ActionSelectNodes(DataFlow* graph, const std::vector<std::shared_ptr<Node>>& nodes);
};
class ActionDeSelectNodes: public ActionDataFlow {
public:
	ActionDeSelectNodes(DataFlow* graph, const std::vector<std::shared_ptr<Node>>& nodes);
};
class ActionSelectConnections: public ActionDataFlow {
public:
	ActionSelectConnections(DataFlow* graph, const std::vector<std::shared_ptr<Connection>>& connections);
};
class ActionDeSelectConnections: public ActionDataFlow {
public:
	ActionDeSelectConnections(DataFlow* graph, const std::vector<std::shared_ptr<Connection>>& connections);
};

class ActionGroupNodes: public ActionDataFlow {
protected:
	std::shared_ptr<Group> group;
public:
	ActionGroupNodes(DataFlow* graph, const std::vector<std::shared_ptr<Node>>& nodes);
};
class ActionUnGroupNodes: public ActionDataFlow {
protected:
	std::vector<std::shared_ptr<Node>> ungrouped;
public:
	ActionUnGroupNodes(DataFlow* graph, const std::vector<std::shared_ptr<Node>>& nodes);
};
struct CoordScalePX {
	pixel2 value;
	Node* node;
	std::string type() const {
		return "CoordScalePX";
	}
	template<class Archive> void serialize(Archive& archive) {
		archive(cereal::make_nvp(type(), value));
	}
	CoordScalePX(float x, float y, Node* node) :
			value(x, y), node(node) {
	}
	pixel2 toPixels(pixel2 screenSize, double2 dpmm, double pixelRatio) const {
		return value * node->getParentFlow()->getScale();
	}
	std::string toString() const {
		return MakeString() << value;
	}
};
struct CoordScalePerPX {
	pixel2 percent;
	pixel2 value;
	Node* node;
	std::string type() const {
		return "CoordScalePerPX";
	}
	template<class Archive> void serialize(Archive& archive) {
		archive(cereal::make_nvp(type(), value));
	}
	CoordScalePerPX(float px, float py, float x, float y, Node* node) :
			percent(px, py), value(x, y), node(node) {

	}
	pixel2 toPixels(pixel2 screenSize, double2 dpmm, double pixelRatio) const {
		float sc = node->getParentFlow()->getScale();
		return pixel2(sc * (value.x + screenSize.x * percent.x), sc * (value.y + screenSize.y * percent.y));

	}
	std::string toString() const {
		return MakeString() << value;
	}
};
template<class C, class R> std::basic_ostream<C, R> & operator <<(std::basic_ostream<C, R> & ss, const Connection& line) {
	return ss << "[" << ((line.source->getNode() == nullptr) ? "NULL" : line.source->getReferenceId())<< "->"
			<< ((line.destination->getNode() == nullptr) ? "NULL" : line.destination->getReferenceId()) << "]";
}
template<class C, class R> std::basic_ostream<C, R> & operator <<(std::basic_ostream<C, R> & ss, const Relationship& line) {
	return ss << "[" << line.subject->getLabel() << " <" << line.predicate->getName() << "> " << line.object->getLabel() << "]";
}
template<class C, class R> std::basic_ostream<C, R> & operator <<(std::basic_ostream<C, R> & ss, const Port& port) {
	ss << "[" << port.getReferenceId() << " (" << port.getConnections().size() << ")";
	if (port.getProxyIn().get() != nullptr) {
		ss << "|<PROXY_IN> " << port.getProxyIn()->getReferenceId();
	}
	if (port.getProxyOut().get() != nullptr) {
		ss << "|<PROXY_OUT> " << port.getProxyOut()->getReferenceId();
	}
	ss << "]";
	return ss;
}
template<class C, class R> std::basic_ostream<C, R> & operator <<(std::basic_ostream<C, R> & ss, const Node& group) {
	ss << group.getLabel()<<" ["<<group.getId()<<"] Input Ports: " << group.getInputPorts().size() << " Output Ports: " << group.getOutputPorts().size() << std::endl;
	for (std::shared_ptr<Port> port : group.getInputPorts()) {
		if (port->isConnected())
			ss << "Input: " << *port << std::endl;
	}
	for (std::shared_ptr<Port> port : group.getOutputPorts()) {
		if (port->isConnected())
			ss << "Output: " << *port << std::endl;
	}
	if (group.getInputPort().get() != nullptr)
		ss << "Node Input: " << *group.getInputPort() << std::endl;
	if (group.getOutputPort().get() != nullptr)
		ss << "Node Output: " << *group.getOutputPort() << std::endl;
	return ss;
}
template<class C, class R> std::basic_ostream<C, R> & operator <<(std::basic_ostream<C, R> & ss, const Group& group) {
	ss << group.getLabel()<<" ["<<group.getId()<<"] Input Ports: " << group.getInputPorts().size() << " Output Ports: " << group.getOutputPorts().size() << std::endl;
	for (std::shared_ptr<Port> port : group.getInputPorts()) {
		if (port->isConnected())
			ss << "Input: " << *port << std::endl;
	}
	for (std::shared_ptr<Port> port : group.getOutputPorts()) {
		if (port->isConnected())
			ss << "Output: " << *port << std::endl;
	}
	for (std::shared_ptr<Connection> connection : group.connections) {
		ss << "Connection: " << *connection << std::endl;
	}
	for (std::shared_ptr<Relationship> relationship : group.relationships) {
		ss << "Relationship: " << *relationship << std::endl;
	}
	return ss;
}

std::shared_ptr<Connection> MakeConnection(const std::shared_ptr<Port>& source, const std::shared_ptr<Port>& destination);
std::shared_ptr<Relationship> MakeRelationship(const std::shared_ptr<Node>& subject, const std::shared_ptr<Predicate>& predicate,
		const std::shared_ptr<Node>& object);
std::shared_ptr<Relationship> MakeRelationship(const std::shared_ptr<Node>& subject, const std::string& predicate, const std::shared_ptr<Node>& object);
std::shared_ptr<Data> MakeDataNode(const std::string& name, const std::string& label, const pixel2& pos);
std::shared_ptr<Data> MakeDataNode(const std::string& name, const pixel2& pos);
std::shared_ptr<Data> MakeDataNode(const std::string& name, const std::string& label);
std::shared_ptr<Data> MakeDataNode(const std::string& name);

std::shared_ptr<Compute> MakeComputeNode(const std::string& name, const std::string& label, const pixel2& pos);
std::shared_ptr<Compute> MakeComputeNode(const std::string& name, const pixel2& pos);
std::shared_ptr<Compute> MakeComputeNode(const std::string& name, const std::string& label);
std::shared_ptr<Compute> MakeComputeNode(const std::string& name);

std::shared_ptr<Group> MakeGroupNode(const std::string& name, const std::string& label, const pixel2& pos);
std::shared_ptr<Group> MakeGroupNode(const std::string& name, const pixel2& pos);
std::shared_ptr<Group> MakeGroupNode(const std::string& name, const std::string& label);
std::shared_ptr<Group> MakeGroupNode(const std::string& name);

std::shared_ptr<View> MakeViewNode(const std::string& name, const std::string& label, const pixel2& pos);
std::shared_ptr<View> MakeViewNode(const std::string& name, const pixel2& pos);
std::shared_ptr<View> MakeViewNode(const std::string& name, const std::string& label);
std::shared_ptr<View> MakeViewNode(const std::string& name);

std::shared_ptr<Destination> MakeDestinationNode(const std::string& name, const std::string& label, const pixel2& pos);
std::shared_ptr<Destination> MakeDestinationNode(const std::string& name, const pixel2& pos);
std::shared_ptr<Destination> MakeDestinationNode(const std::string& name, const std::string& label);
std::shared_ptr<Destination> MakeDestinationNode(const std::string& name);

std::shared_ptr<Source> MakeSourceNode(const std::string& name, const std::string& label, const pixel2& pos);
std::shared_ptr<Source> MakeSourceNode(const std::string& name, const pixel2& pos);
std::shared_ptr<Source> MakeSourceNode(const std::string& name, const std::string& label);
std::shared_ptr<Source> MakeSourceNode(const std::string& name);

std::shared_ptr<DataFlow> MakeDataFlow(const std::string& name, const AUnit2D& pos, const AUnit2D& dims);
std::shared_ptr<InputPort> MakeInputPort(const std::string& name);
std::shared_ptr<OutputPort> MakeOutputPort(const std::string& name);
std::shared_ptr<ParentPort> MakeParentPort(const std::string& name);
std::shared_ptr<ChildPort> MakeChildPort(const std::string& name);
typedef std::shared_ptr<Node> NodePtr;
typedef std::shared_ptr<View> ViewPtr;
typedef std::shared_ptr<Compute> ComputePtr;
typedef std::shared_ptr<Data> DataPtr;
typedef std::shared_ptr<Destination> DestinationPtr;
typedef std::shared_ptr<Source> SourcePtr;
typedef std::shared_ptr<Group> GroupPtr;
typedef std::shared_ptr<DataFlow> DataFlowPtr;
typedef std::shared_ptr<Port> PortPtr;
typedef std::shared_ptr<InputPort> InputPortPtr;
typedef std::shared_ptr<OutputPort> OutputPortPtr;
typedef std::shared_ptr<MultiPort> MultiPortPtr;
typedef std::shared_ptr<Connection> ConnectionPtr;
typedef std::shared_ptr<ConnectionBundle> ConnectionBundlePtr;
typedef std::shared_ptr<Relationship> RelationshipPtr;
typedef std::shared_ptr<Predicate> PredicatePtr;
typedef std::shared_ptr<Packet> PacketPtr;
typedef std::shared_ptr<NodeIcon> NodeIconPtr;
typedef std::shared_ptr<ParentPort> ParentPortPtr;
typedef std::shared_ptr<ChildPort> ChildPortPtr;
}
}
#endif /* INCLUDE_CORE_ALLOYUALGRAPH_H_ */
