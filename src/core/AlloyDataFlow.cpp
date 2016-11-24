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
#include "AlloyDataFlow.h"
#include "AlloyApplication.h"
#include "AlloyDrawUtil.h"
#include "ForceDirectedGraph.h"
namespace aly {
	namespace dataflow {
		const int MultiPort::FrontIndex = std::numeric_limits<int>::min();
		const int MultiPort::BackIndex = std::numeric_limits<int>::max();
		const pixel2 Node::DIMENSIONS = pixel2(80, 80);
		const pixel2 InputPort::DIMENSIONS = pixel2(12, 12);
		const pixel2 OutputPort::DIMENSIONS = pixel2(12, 11);
		const pixel2 ParentPort::DIMENSIONS = pixel2(12, 12);
		const pixel2 ChildPort::DIMENSIONS = pixel2(11, 12);
		const float NODE_SATURATION = 0.6f;
		const float NODE_LUMINANCE = 0.65f;
		const float NODE_ALPHA = 0.75f;
		const Color View::COLOR = HSVAtoColor(HSVA(30 / 360.0f, NODE_SATURATION, 0.5f * NODE_LUMINANCE, 1.0f));
		const Color Compute::COLOR = HSVAtoColor(HSVA(0.0f, NODE_SATURATION, NODE_LUMINANCE, 1.0f));
		const Color Data::COLOR = HSVAtoColor(HSVA(60.0f / 360.0f, NODE_SATURATION, NODE_LUMINANCE, 1.0f));
		const Color Group::COLOR = HSVAtoColor(HSVA(300.0f / 360.0f, NODE_SATURATION, 0.5f * NODE_LUMINANCE, 1.0f));
		const Color Destination::COLOR = HSVAtoColor(HSVA(120.0f / 360.0f, NODE_SATURATION, NODE_LUMINANCE, 1.0f));
		const Color Source::COLOR = HSVAtoColor(HSVA(225.0f / 360.0f, NODE_SATURATION, NODE_LUMINANCE, 1.0f));
		std::shared_ptr<Group> DataFlow::clipboard = std::shared_ptr<Group>();
		std::shared_ptr<InputPort> MakeInputPort(const std::string& name) {
			return InputPortPtr(new InputPort(name));
		}
		std::shared_ptr<OutputPort> MakeOutputPort(const std::string& name) {
			return OutputPortPtr(new OutputPort(name));
		}
		std::shared_ptr<ParentPort> MakeParentPort(const std::string& name) {
			return ParentPortPtr(new ParentPort(name));
		}
		std::shared_ptr<ChildPort> MakeChildPort(const std::string& name) {
			return ChildPortPtr(new ChildPort(name));
		}
		std::shared_ptr<DataFlow> MakeDataFlow(const std::string& name, const AUnit2D& pos, const AUnit2D& dims) {
			return DataFlowPtr(new DataFlow(name, pos, dims));
		}
		std::shared_ptr<Connection> MakeConnection(const std::shared_ptr<Port>& source, const std::shared_ptr<Port>& destination) {
			ConnectionPtr connection = ConnectionPtr(new Connection(source, destination));
			if (!source->connect(connection)) {
				throw std::runtime_error(aly::MakeString() << "Cannot connect to source twice. " << *connection);
			}
			if (!destination->connect(connection)) {
				throw std::runtime_error(aly::MakeString() << "Cannot connect to destination twice. " << *connection);
			}
			return connection;
		}
		std::shared_ptr<Relationship> MakeRelationship(const std::shared_ptr<Node>& subject, const std::shared_ptr<Predicate>& predicate,
				const std::shared_ptr<Node>& object) {
			return RelationshipPtr(new Relationship(subject, predicate, object));
		}
		std::shared_ptr<Relationship> MakeRelationship(const std::shared_ptr<Node>& subject, const std::string& name, const std::shared_ptr<Node>& object) {
			return RelationshipPtr(new Relationship(subject, PredicatePtr(new Predicate(name)), object));
		}
		std::shared_ptr<Data> MakeDataNode(const std::string& name, const std::string& label, const pixel2& pos) {
			return DataPtr(new Data(name, label, pos));
		}
		std::shared_ptr<Data> MakeDataNode(const std::string& name, const pixel2& pos) {
			return DataPtr(new Data(name, pos));
		}
		std::shared_ptr<Data> MakeDataNode(const std::string& name, const std::string& label) {
			return DataPtr(new Data(name, label, pixel2(0.0f)));
		}
		std::shared_ptr<Data> MakeDataNode(const std::string& name) {
			return DataPtr(new Data(name, pixel2(0.0f)));
		}

		std::shared_ptr<View> MakeViewNode(const std::string& name, const std::string& label, const pixel2& pos) {
			return ViewPtr(new View(name, label, pos));
		}
		std::shared_ptr<View> MakeViewNode(const std::string& name, const pixel2& pos) {
			return ViewPtr(new View(name, pos));
		}
		std::shared_ptr<View> MakeViewNode(const std::string& name, const std::string& label) {
			return ViewPtr(new View(name, label, pixel2(0.0f)));
		}
		std::shared_ptr<View> MakeViewNode(const std::string& name) {
			return ViewPtr(new View(name, pixel2(0.0f)));
		}

		std::shared_ptr<Compute> MakeComputeNode(const std::string& name, const std::string& label, const pixel2& pos) {
			return ComputePtr(new Compute(name, label, pos));
		}
		std::shared_ptr<Compute> MakeComputeNode(const std::string& name, const pixel2& pos) {
			return ComputePtr(new Compute(name, pos));
		}
		std::shared_ptr<Compute> MakeComputeNode(const std::string& name, const std::string& label) {
			return ComputePtr(new Compute(name, label, pixel2(0.0f)));
		}
		std::shared_ptr<Compute> MakeComputeNode(const std::string& name) {
			return ComputePtr(new Compute(name, pixel2(0.0f)));
		}

		std::shared_ptr<Group> MakeGroupNode(const std::string& name, const std::string& label, const pixel2& pos) {
			return GroupPtr(new Group(name, label, pos));
		}
		std::shared_ptr<Group> MakeGroupNode(const std::string& name, const pixel2& pos) {
			return GroupPtr(new Group(name, pos));
		}
		std::shared_ptr<Group> MakeGroupNode(const std::string& name, const std::string& label) {
			return GroupPtr(new Group(name, label, pixel2(0.0f)));
		}
		std::shared_ptr<Group> MakeGroupNode(const std::string& name) {
			return GroupPtr(new Group(name, pixel2(0.0f)));
		}
		std::shared_ptr<Source> MakeSourceNode(const std::string& name, const std::string& label, const pixel2& pos) {
			return SourcePtr(new Source(name, label, pos));
		}
		std::shared_ptr<Source> MakeSourceNode(const std::string& name, const pixel2& pos) {
			return SourcePtr(new Source(name, pos));
		}
		std::shared_ptr<Source> MakeSourceNode(const std::string& name, const std::string& label) {
			return SourcePtr(new Source(name, label, pixel2(0.0f)));
		}
		std::shared_ptr<Source> MakeSourceNode(const std::string& name) {
			return SourcePtr(new Source(name, pixel2(0.0f)));
		}

		std::shared_ptr<Destination> MakeDestinationNode(const std::string& name, const std::string& label, const pixel2& pos) {
			return DestinationPtr(new Destination(name, label, pos));
		}
		std::shared_ptr<Destination> MakeDestinationNode(const std::string& name, const pixel2& pos) {
			return DestinationPtr(new Destination(name, pos));
		}
		std::shared_ptr<Destination> MakeDestinationNode(const std::string& name, const std::string& label) {
			return DestinationPtr(new Destination(name, label, pixel2(0.0f)));
		}
		std::shared_ptr<Destination> MakeDestinationNode(const std::string& name) {
			return DestinationPtr(new Destination(name, pixel2(0.0f)));
		}

		DataFlow::~DataFlow() {
			if (forceSim.get() != nullptr)
				forceSim->stop();
		}
		void NodeIcon::draw(AlloyContext* context) {
			NVGcontext* nvg = context->nvgContext;
			box2px bounds = getBounds();
			pixel lineWidth = 0;
			nvgStrokeWidth(nvg, lineWidth);
			static std::vector<float2> pentagon;
			if (pentagon.size() == 0) {
				pentagon.resize(6);
				for (int k = 0; k < 5; k++) {
					pentagon[k] = float2(std::sin(ALY_PI * 2 * k / 5.0f), -std::cos(ALY_PI * 2 * k / 5.0f));
				}
				pentagon[5] = float2(0.0f);
			}
			if (context->isMouseOver(this)) {
				nvgFillColor(nvg, backgroundColor->toLighter(0.25f));
				//context->setCursor(&Cursor::Position);
			} else {
				nvgFillColor(nvg, *backgroundColor);
			}
			Color strokeColor;
			if (selected) {
				nvgStrokeWidth(nvg, borderScale * 6.0f);
				lineWidth = 6 * borderScale;
				nvgStrokeColor(nvg, strokeColor = context->theme.LIGHTEST);
			} else {
				nvgStrokeWidth(nvg, borderScale * 4.0f);
				lineWidth = 4 * borderScale;
				nvgStrokeColor(nvg, strokeColor = context->theme.LIGHTEST.toDarker(0.8f));
			}
			if (shape == NodeShape::Circle) {
				nvgBeginPath(nvg);
				nvgEllipse(nvg, bounds.position.x + bounds.dimensions.x * 0.5f, bounds.position.y + bounds.dimensions.y * 0.5f,
						0.5f * bounds.dimensions.y - lineWidth * 0.5f, 0.5f * bounds.dimensions.y - lineWidth * 0.5f);

				nvgFill(nvg);
				nvgStroke(nvg);
			} else if (shape == NodeShape::CircleGroup) {
				nvgBeginPath(nvg);
				nvgEllipse(nvg, bounds.position.x + bounds.dimensions.x * 0.5f, bounds.position.y + bounds.dimensions.y * 0.5f,
						0.5f * bounds.dimensions.y - lineWidth * 0.5f, 0.5f * bounds.dimensions.y - lineWidth * 0.5f);

				nvgFill(nvg);
				nvgStroke(nvg);
				nvgFillColor(nvg, strokeColor);
				float cx = bounds.position.x + bounds.dimensions.x * 0.5f;
				float cy = bounds.position.y + bounds.dimensions.y * 0.5f;
				float r = borderScale * 4.0f;
				float rx = 0.5f * (0.5f * bounds.dimensions.x);
				float ry = 0.5f * (0.5f * bounds.dimensions.y);
				for (int k = 0; k < (int) pentagon.size(); k++) {
					nvgBeginPath(nvg);
					float2 pt = pentagon[k];
					float x = cx + rx * pt.x;
					float y = cy + ry * pt.y;
					nvgCircle(nvg, x, y, r);
					nvgFill(nvg);
				}
			} else if (shape == NodeShape::Square) {
				nvgBeginPath(nvg);
				nvgRoundedRect(nvg, bounds.position.x + lineWidth * 0.5f, bounds.position.y + lineWidth * 0.5f, bounds.dimensions.x - lineWidth,
						bounds.dimensions.y - lineWidth, bounds.dimensions.x * 0.25f);
				nvgFill(nvg);
				nvgStroke(nvg);
			} else if (shape == NodeShape::Triangle) {
				nvgBeginPath(nvg);
				nvgLineJoin(nvg, NVG_ROUND);
				nvgMoveTo(nvg, bounds.position.x + bounds.dimensions.x * 0.5f, bounds.position.y + bounds.dimensions.y - lineWidth);
				nvgLineTo(nvg, bounds.position.x + lineWidth * 0.5f, bounds.position.y + lineWidth * 0.5f);
				nvgLineTo(nvg, bounds.position.x + bounds.dimensions.x - lineWidth, bounds.position.y + lineWidth * 0.5f);
				nvgClosePath(nvg);
				nvgFill(nvg);
				nvgStroke(nvg);
			} else if (shape == NodeShape::Hexagon) {
				nvgBeginPath(nvg);
				nvgLineJoin(nvg, NVG_ROUND);
				float cx = bounds.position.x + bounds.dimensions.x * 0.5f;
				float cy = bounds.position.y + bounds.dimensions.y * 0.5f;
				static const float SCALE = 1.0f / std::sqrt(0.75f);
				float rx = (0.5f * bounds.dimensions.x - lineWidth * 0.5f) * SCALE;
				float ry = (0.5f * bounds.dimensions.y - lineWidth * 0.5f);
				nvgMoveTo(nvg, cx + rx, cy);
				nvgLineTo(nvg, cx + rx * 0.5f, cy - ry);
				nvgLineTo(nvg, cx - rx * 0.5f, cy - ry);
				nvgLineTo(nvg, cx - rx * 0.5f, cy - ry);
				nvgLineTo(nvg, cx - rx, cy);
				nvgLineTo(nvg, cx - rx * 0.5f, cy + ry);
				nvgLineTo(nvg, cx + rx * 0.5f, cy + ry);
				nvgClosePath(nvg);
				nvgFill(nvg);
				nvgStroke(nvg);
			}
		}
		std::string Node::MakeID(int len) {
			std::stringstream ss;
			static const char lookUp[33] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ012345";
			for (int i = 0; i < len; i++) {
				ss << lookUp[RandomUniform(0, 31)];
			}
			return ss.str();
		}
		void Node::setup() {
			borderWidth = UnitPX(4.0f);
			fontSize = 20.0f;
			id="n"+MakeID();
		}
		ForceSimulatorPtr Node::getForceSimulator() {
			if (parentFlow == nullptr) {
				throw std::runtime_error(aly::MakeString() << getName() << ":: Could not get force simulator because parent flow is null.");
			}
			return parentFlow->getForceSimulator();
		}
		box2px Node::getObstacleBounds() const {
			box2px box = nodeIcon->getBounds(false);
			box.position += getForceOffset();
			return box;
		}
		pixel2 Node::getForceOffset() const {
			if (parentFlow == nullptr) {
				throw std::runtime_error(aly::MakeString() << getName() << ":: Could not get force offset because parent flow is null.");
			}
			return parentFlow->getScale() * forceItem->location - centerOffset;
		}
		float2 Port::getLocation() const {
			return getBounds(false).center() + parent->getForceOffset();
		}
		void View::setup() {
			setOrientation(Orientation::Horizontal, pixel2(0, 0));
			NVGcontext* nvg = AlloyApplicationContext()->nvgContext;
			nvgFontSize(nvg, fontSize);
			nvgFontFaceId(nvg, AlloyApplicationContext()->getFont(FontType::Bold)->handle);
			textWidth = nvgTextBounds(nvg, 0, 0, label.c_str(), nullptr, nullptr);
			CompositePtr iconContainer = MakeComposite("Icon Container", CoordPX(0.0f, 0.0f),
					CoordPX(Node::DIMENSIONS.x - InputPort::DIMENSIONS.x - OutputPort::DIMENSIONS.x, Node::DIMENSIONS.y));

			CompositePtr labelContainer = MakeComposite("label Container", CoordPX(0.0f, 0.0f), CoordPerPX(1.0f, 0.0f, 0.0f, Node::DIMENSIONS.y));
			nodeIcon = NodeIconPtr(
					new NodeIcon("Icon", CoordPX(0.0f, InputPort::DIMENSIONS.y + 1.0f),
							CoordPerPX(1.0f, 1.0f, 0.0f, -OutputPort::DIMENSIONS.y - InputPort::DIMENSIONS.y - 2.0f)));
			resizeFunc =
					[this,iconContainer,labelContainer](float scale) {
						const float nx=scale*Node::DIMENSIONS.x;
						const float ny=scale*Node::DIMENSIONS.y;
						const float ix=scale*InputPort::DIMENSIONS.x;
						const float iy=scale*InputPort::DIMENSIONS.y;
						const float ox=scale*OutputPort::DIMENSIONS.x;
						const float oy=scale*OutputPort::DIMENSIONS.y;

						this->position=CoordPX(0.0f,0.0f);
						this->dimensions = CoordPX(
								std::max(std::max(std::max(scale*(textWidth + 10.0f),0.5f*nx),2.0f+ inputPorts.size()* (ix + 2.0f)),2.0f+ outputPorts.size()* (ox + 2.0f)),ny);
						iconContainer->position=CoordPX(0.0f,0.0f);
						iconContainer->dimensions=CoordPX(nx - ix- ox, ny);
						labelContainer->position=CoordPX(nx, 0.0f);
						labelContainer->dimensions=CoordPerPX(1.0f, 0.0f, 0.0f, ny);
						inputPort->position = CoordPerPX(0.5f, 0.0f,-ix * 0.5f, 0.0f);
						nodeIcon->position=CoordPX(0.0f, iy + 1.0f);
						nodeIcon->dimensions=CoordPerPX(1.0f, 1.0f, 0.0f,-oy - iy- 2.0f);
						inputPortComposite->position=CoordPX(0.0f, iy);
						inputPortComposite->dimensions=CoordPX(0.0f, iy);
						outputPortComposite->position=CoordPerPX(0.0f, 1.0f, 0.0f, -2 * oy);
						outputPortComposite->dimensions=CoordPX(0.0f, oy);
						labelRegion->fontSize=UnitPX(scale*fontSize);
						nodeIcon->borderScale=scale;
						labelRegion->position=CoordPX(0.0f, 2 * iy + 1.0f);
						labelRegion->dimensions=CoordPerPX(0.0f, 1.0f,std::max(scale*(10 + textWidth), 0.5f * nx),-2 * oy- 2 * iy- 2.0f);
					};
			nodeIcon->setAspectRule(AspectRule::FixedHeight);
			nodeIcon->setAspectRatio(1.0f);
			inputPort = MakeInputPort("Input");
			inputPort->position = CoordPerPX(0.5f, 0.0f, -InputPort::DIMENSIONS.x * 0.5f, 0.0f);
			inputPort->setParent(this);
			iconContainer->add(inputPort);
			iconContainer->add(nodeIcon);
			labelRegion = ModifiableLabelPtr(
					new ModifiableLabel(name, CoordPX(0.0f, 2 * InputPort::DIMENSIONS.y + 1.0f),
							CoordPerPX(0.0f, 1.0f, std::max(10 + textWidth, 0.5f * Node::DIMENSIONS.x),
									-2 * OutputPort::DIMENSIONS.y - 2 * InputPort::DIMENSIONS.y - 2.0f)));
			labelRegion->setValue(label);
			labelRegion->setAlignment(HorizontalAlignment::Left, VerticalAlignment::Middle);
			labelRegion->fontSize = UnitPX(fontSize);
			labelRegion->fontType = FontType::Bold;
			labelRegion->backgroundColor = MakeColor(0, 0, 0, 0);
			labelRegion->borderColor = MakeColor(0, 0, 0, 0);
			labelRegion->borderWidth = UnitPX(0.0f);
			labelRegion->onTextEntered = [this](TextField* field) {
				AUnit1D fz=labelRegion->fontSize;
				labelRegion->fontSize=UnitPX(fontSize);
				textWidth = labelRegion->getTextDimensions(AlloyApplicationContext().get()).x;
				labelRegion->fontSize=fz;
				label=field->getValue();
				if(parentFlow->onNodeAction)parentFlow->onNodeAction(this,NodeAction::Modify);
			};
			labelContainer->add(labelRegion);
			inputPortComposite = CompositePtr(new Composite("Input Ports", CoordPX(0.0f, InputPort::DIMENSIONS.y), CoordPX(0.0f, InputPort::DIMENSIONS.y)));
			outputPortComposite = CompositePtr(
					new Composite("Output Ports", CoordPerPX(0.0f, 1.0f, 0.0f, -2 * OutputPort::DIMENSIONS.y), CoordPX(0.0f, OutputPort::DIMENSIONS.y)));

			inputPortComposite->setOrientation(Orientation::Horizontal, pixel2(2, 0), pixel2(2, 0));
			outputPortComposite->setOrientation(Orientation::Horizontal, pixel2(2, 0), pixel2(2, 0));

			labelContainer->add(inputPortComposite);
			labelContainer->add(outputPortComposite);

			Composite::add(iconContainer);
			Composite::add(labelContainer);
			setRoundCorners(true);
			nodeIcon->backgroundColor = MakeColor(COLOR);
			nodeIcon->setShape(NodeShape::Square);
			nodeIcon->borderWidth = borderWidth;
			forceItem->buoyancy = 0;

		}
		void Node::setLabel(const std::string& label,bool notify) {
			if (labelRegion.get() != nullptr && labelRegion->getValue() != label) {
				labelRegion->setValue(label);
				if(notify){
					labelRegion->onTextEntered(labelRegion.get());
				} else {
					AUnit1D fz=labelRegion->fontSize;
					labelRegion->fontSize=UnitPX(fontSize);
					textWidth = labelRegion->getTextDimensions(AlloyApplicationContext().get()).x;
					labelRegion->fontSize=fz;
					this->label=label;
				}
			} else {
				this->label = label;
			}
		}
		void Group::setup() {
			setOrientation(Orientation::Horizontal, pixel2(0, 0));
			NVGcontext* nvg = AlloyApplicationContext()->nvgContext;
			nvgFontSize(nvg, fontSize);
			nvgFontFaceId(nvg, AlloyApplicationContext()->getFont(FontType::Bold)->handle);
			textWidth = nvgTextBounds(nvg, 0, 0, label.c_str(), nullptr, nullptr);
			CompositePtr iconContainer = MakeComposite("Icon Container", CoordPX(0.0f, 0.0f),
					CoordPX(Node::DIMENSIONS.x - InputPort::DIMENSIONS.x - OutputPort::DIMENSIONS.x, Node::DIMENSIONS.y));

			CompositePtr labelContainer = MakeComposite("label Container", CoordPX(Node::DIMENSIONS.x, 0.0f), CoordPerPX(1.0f, 0.0f, 0.0f, Node::DIMENSIONS.y));

			nodeIcon = NodeIconPtr(
					new NodeIcon("Icon", CoordPX(0.0f, InputPort::DIMENSIONS.y + 1.0f),
							CoordPerPX(1.0f, 1.0f, 0.0f, -OutputPort::DIMENSIONS.y - InputPort::DIMENSIONS.y - 2.0f)));
			resizeFunc =
					[this,iconContainer,labelContainer](float scale) {
						const float nx=scale*Node::DIMENSIONS.x;
						const float ny=scale*Node::DIMENSIONS.y;
						const float ix=scale*InputPort::DIMENSIONS.x;
						const float iy=scale*InputPort::DIMENSIONS.y;
						const float ox=scale*OutputPort::DIMENSIONS.x;
						const float oy=scale*OutputPort::DIMENSIONS.y;

						this->position=CoordPX(0.0f,0.0f);
						this->dimensions = CoordPX(
								std::max(std::max(std::max(scale*(textWidth + 10.0f),0.5f*nx),2.0f+ inputPorts.size()* (ix + 2.0f)),2.0f+ outputPorts.size()* (ox + 2.0f)),ny);
						iconContainer->position=CoordPX(0.0f,0.0f);
						iconContainer->dimensions=CoordPX(nx - ix- ox, ny);
						labelContainer->position=CoordPX(nx, 0.0f);
						labelContainer->dimensions=CoordPerPX(1.0f, 0.0f, 0.0f, ny);
						nodeIcon->position=CoordPX(0.0f, iy + 1.0f);
						nodeIcon->dimensions=CoordPerPX(1.0f, 1.0f, 0.0f,-oy - iy- 2.0f);
						inputPortComposite->position=CoordPX(0.0f, iy);
						inputPortComposite->dimensions=CoordPX(0.0f, iy);
						outputPortComposite->position=CoordPerPX(0.0f, 1.0f, 0.0f, -2 * oy);
						outputPortComposite->dimensions=CoordPX(0.0f, oy);
						labelRegion->position=CoordPX(0.0f, 2 * iy + 1.0f);
						labelRegion->fontSize=UnitPX(scale*fontSize);
						nodeIcon->borderScale=scale;
						labelRegion->dimensions=CoordPerPX(0.0f, 1.0f,std::max(scale*(10 + textWidth), 0.5f * nx),-2 * oy- 2 * iy- 2.0f);
					};
			nodeIcon->setAspectRule(AspectRule::FixedHeight);
			nodeIcon->setAspectRatio(1.0f);
			iconContainer->add(nodeIcon);
			labelRegion = ModifiableLabelPtr(
					new ModifiableLabel(name, CoordPX(0.0f, 2 * InputPort::DIMENSIONS.y + 1.0f),
							CoordPerPX(0.0f, 1.0f, std::max(10 + textWidth, 0.5f * Node::DIMENSIONS.x),
									-2 * OutputPort::DIMENSIONS.y - 2 * InputPort::DIMENSIONS.y - 2.0f)));
			labelRegion->setValue(label);
			labelRegion->setAlignment(HorizontalAlignment::Left, VerticalAlignment::Middle);
			labelRegion->fontSize = UnitPX(fontSize);
			labelRegion->fontType = FontType::Bold;
			labelRegion->backgroundColor = MakeColor(0, 0, 0, 0);
			labelRegion->borderColor = MakeColor(0, 0, 0, 0);
			labelRegion->borderWidth = UnitPX(0.0f);
			labelRegion->onTextEntered = [this](TextField* field) {
				AUnit1D fz=labelRegion->fontSize;
				labelRegion->fontSize=UnitPX(fontSize);
				textWidth = labelRegion->getTextDimensions(AlloyApplicationContext().get()).x;
				labelRegion->fontSize=fz;
				label=field->getValue();
				if(parentFlow->onNodeAction)parentFlow->onNodeAction(this,NodeAction::Modify);
			};
			labelContainer->add(labelRegion);
			inputPortComposite = CompositePtr(new Composite("Input Ports", CoordPX(0.0f, InputPort::DIMENSIONS.y), CoordPX(0.0f, InputPort::DIMENSIONS.y)));
			outputPortComposite = CompositePtr(
					new Composite("Output Ports", CoordPerPX(0.0f, 1.0f, 0.0f, -2 * OutputPort::DIMENSIONS.y), CoordPX(0.0f, OutputPort::DIMENSIONS.y)));

			inputPortComposite->setOrientation(Orientation::Horizontal, pixel2(2, 0), pixel2(2, 0));
			outputPortComposite->setOrientation(Orientation::Horizontal, pixel2(2, 0), pixel2(2, 0));
			labelContainer->add(inputPortComposite);
			labelContainer->add(outputPortComposite);

			Composite::add(iconContainer);
			Composite::add(labelContainer);
			setRoundCorners(true);
			nodeIcon->setShape(NodeShape::CircleGroup);
			nodeIcon->backgroundColor = MakeColor(COLOR);
			nodeIcon->borderWidth = borderWidth;
			forceItem->buoyancy = 0;

		}
		void Data::setup() {

			setOrientation(Orientation::Horizontal, pixel2(0, 0));
			NVGcontext* nvg = AlloyApplicationContext()->nvgContext;
			nvgFontSize(nvg, fontSize);
			nvgFontFaceId(nvg, AlloyApplicationContext()->getFont(FontType::Bold)->handle);
			textWidth = nvgTextBounds(nvg, 0, 0, label.c_str(), nullptr, nullptr);
			CompositePtr iconContainer = MakeComposite("Icon Container", CoordPX(0.0f, 0.0f),
					CoordPX(Node::DIMENSIONS.x - InputPort::DIMENSIONS.x - OutputPort::DIMENSIONS.x, Node::DIMENSIONS.y));

			CompositePtr labelContainer = MakeComposite("label Container", CoordPX(Node::DIMENSIONS.x, 0.0f), CoordPerPX(1.0f, 0.0f, 0.0f, Node::DIMENSIONS.y));

			nodeIcon = NodeIconPtr(
					new NodeIcon("Icon", CoordPX(0.0f, InputPort::DIMENSIONS.y + 1.0f),
							CoordPerPX(1.0f, 1.0f, 0.0f, -OutputPort::DIMENSIONS.y - InputPort::DIMENSIONS.y - 2.0f)));

			labelRegion = ModifiableLabelPtr(
					new ModifiableLabel(name, CoordPX(0.0f, 2 * InputPort::DIMENSIONS.y + 1.0f),
							CoordPerPX(0.0f, 1.0f, std::max(10 + textWidth, 0.5f * Node::DIMENSIONS.x),
									-2 * OutputPort::DIMENSIONS.y - 2 * InputPort::DIMENSIONS.y - 2.0f)));

			inputPortComposite = CompositePtr(new Composite("Input Ports", CoordPX(0.0f, InputPort::DIMENSIONS.y), CoordPX(0.0f, InputPort::DIMENSIONS.y)));
			outputPortComposite = CompositePtr(
					new Composite("Output Ports", CoordPerPX(0.0f, 1.0f, 0.0f, -2 * OutputPort::DIMENSIONS.y), CoordPX(0.0f, OutputPort::DIMENSIONS.y)));
			inputPort = MakeInputPort("Input");
			outputPort = MakeOutputPort("Output");
			inputPort->position = CoordPerPX(0.5f, 0.0f, -InputPort::DIMENSIONS.x * 0.5f, 0.0f);
			outputPort->position = CoordPerPX(0.5f, 1.0f, -OutputPort::DIMENSIONS.x * 0.5f, -OutputPort::DIMENSIONS.y);
			resizeFunc =
					[this,iconContainer,labelContainer](float scale) {
						const float nx=scale*Node::DIMENSIONS.x;
						const float ny=scale*Node::DIMENSIONS.y;
						const float ix=scale*InputPort::DIMENSIONS.x;
						const float iy=scale*InputPort::DIMENSIONS.y;
						const float ox=scale*OutputPort::DIMENSIONS.x;
						const float oy=scale*OutputPort::DIMENSIONS.y;

						this->position=CoordPX(0.0f,0.0f);
						this->dimensions = CoordPX(
								std::max(std::max(std::max(scale*(textWidth + 10.0f),0.5f*nx),2.0f+ inputPorts.size()* (ix + 2.0f)),2.0f+ outputPorts.size()* (ox + 2.0f)),ny);
						iconContainer->position=CoordPX(0.0f,0.0f);
						iconContainer->dimensions=CoordPX(nx - ix- ox, ny);
						labelContainer->position=CoordPX(nx, 0.0f);
						labelContainer->dimensions=CoordPerPX(1.0f, 0.0f, 0.0f, ny);
						inputPort->position = CoordPerPX(0.5f, 0.0f,-ix * 0.5f, 0.0f);
						outputPort->position = CoordPerPX(0.5f, 1.0f,-ox * 0.5f, -oy);
						nodeIcon->position=CoordPX(0.0f, iy + 1.0f);
						nodeIcon->dimensions=CoordPerPX(1.0f, 1.0f, 0.0f,-oy - iy- 2.0f);
						inputPortComposite->position=CoordPX(0.0f, iy);
						inputPortComposite->dimensions=CoordPX(0.0f, iy);
						outputPortComposite->position=CoordPerPX(0.0f, 1.0f, 0.0f, -2 * oy);
						outputPortComposite->dimensions=CoordPX(0.0f, oy);
						labelRegion->position=CoordPX(0.0f, 2 * iy + 1.0f);
						labelRegion->fontSize=UnitPX(scale*fontSize);
						nodeIcon->borderScale=scale;
						labelRegion->dimensions=CoordPerPX(0.0f, 1.0f,std::max(scale*(10 + textWidth), 0.5f * nx),-2 * oy- 2 * iy- 2.0f);
					};
			nodeIcon->setAspectRule(AspectRule::FixedHeight);
			nodeIcon->setAspectRatio(1.0f);

			inputPort->setParent(this);
			outputPort->setParent(this);
			iconContainer->add(inputPort);
			iconContainer->add(outputPort);
			iconContainer->add(nodeIcon);

			labelRegion->setValue(label);
			labelRegion->setAlignment(HorizontalAlignment::Left, VerticalAlignment::Middle);
			labelRegion->fontSize = UnitPX(fontSize);
			labelRegion->fontType = FontType::Bold;
			labelRegion->backgroundColor = MakeColor(0, 0, 0, 0);
			labelRegion->borderColor = MakeColor(0, 0, 0, 0);
			labelRegion->borderWidth = UnitPX(0.0f);
			labelRegion->onTextEntered = [this](TextField* field) {
				AUnit1D fz=labelRegion->fontSize;
				labelRegion->fontSize=UnitPX(fontSize);
				textWidth = labelRegion->getTextDimensions(AlloyApplicationContext().get()).x;
				labelRegion->fontSize=fz;
				label=field->getValue();
				if(parentFlow->onNodeAction)parentFlow->onNodeAction(this,NodeAction::Modify);
			};
			labelContainer->add(labelRegion);

			inputPortComposite->setOrientation(Orientation::Horizontal, pixel2(2, 0), pixel2(2, 0));
			outputPortComposite->setOrientation(Orientation::Horizontal, pixel2(2, 0), pixel2(2, 0));
			labelContainer->add(inputPortComposite);
			labelContainer->add(outputPortComposite);

			Composite::add(iconContainer);
			Composite::add(labelContainer);
			setRoundCorners(true);
			nodeIcon->setShape(NodeShape::Circle);
			nodeIcon->backgroundColor = MakeColor(COLOR);
			nodeIcon->borderWidth = borderWidth;
			forceItem->buoyancy = 0;

		}
		void Compute::setup() {
			setOrientation(Orientation::Horizontal, pixel2(0, 0));
			NVGcontext* nvg = AlloyApplicationContext()->nvgContext;
			nvgFontSize(nvg, fontSize);
			nvgFontFaceId(nvg, AlloyApplicationContext()->getFont(FontType::Bold)->handle);
			textWidth = nvgTextBounds(nvg, 0, 0, label.c_str(), nullptr, nullptr);
			CompositePtr iconContainer = MakeComposite("Icon Container", CoordPX(0.0f, 0.0f),
					CoordPX(Node::DIMENSIONS.x - InputPort::DIMENSIONS.x - OutputPort::DIMENSIONS.x, Node::DIMENSIONS.y));
			CompositePtr labelContainer = MakeComposite("label Container", CoordPX(Node::DIMENSIONS.x, 0.0f), CoordPerPX(1.0f, 0.0f, 0.0f, Node::DIMENSIONS.y));
			nodeIcon = NodeIconPtr(
					new NodeIcon("Icon", CoordPX(0.0f, InputPort::DIMENSIONS.y + 1.0f),
							CoordPerPX(1.0f, 1.0f, 0.0f, -OutputPort::DIMENSIONS.y - InputPort::DIMENSIONS.y - 2.0f)));
			inputPortComposite = CompositePtr(new Composite("Input Ports", CoordPX(0.0f, InputPort::DIMENSIONS.y), CoordPX(0.0f, InputPort::DIMENSIONS.y)));
			outputPortComposite = CompositePtr(
					new Composite("Output Ports", CoordPerPX(0.0f, 1.0f, 0.0f, -2 * OutputPort::DIMENSIONS.y), CoordPX(0.0f, OutputPort::DIMENSIONS.y)));
			inputPort = MakeInputPort("Input");
			inputPort->position = CoordPerPX(0.5f, 0.0f, -InputPort::DIMENSIONS.x * 0.5f, 0.0f);
			labelRegion = ModifiableLabelPtr(
					new ModifiableLabel(name, CoordPX(2.0f, 2 * InputPort::DIMENSIONS.y + 1.0f),
							CoordPerPX(0.0f, 1.0f, std::max(10 + textWidth, 0.5f * Node::DIMENSIONS.x),
									-2 * OutputPort::DIMENSIONS.y - 2 * InputPort::DIMENSIONS.y - 2.0f)));
			resizeFunc =
					[this,iconContainer,labelContainer](float scale) {
						const float nx=scale*Node::DIMENSIONS.x;
						const float ny=scale*Node::DIMENSIONS.y;
						const float ix=scale*InputPort::DIMENSIONS.x;
						const float iy=scale*InputPort::DIMENSIONS.y;
						const float ox=scale*OutputPort::DIMENSIONS.x;
						const float oy=scale*OutputPort::DIMENSIONS.y;

						this->position=CoordPX(0.0f,0.0f);
						this->dimensions = CoordPX(
								std::max(std::max(std::max(scale*(textWidth + 10.0f),0.5f*nx),2.0f+ inputPorts.size()* (ix + 2.0f)),2.0f+ outputPorts.size()* (ox + 2.0f)),ny);
						iconContainer->position=CoordPX(0.0f,0.0f);
						iconContainer->dimensions=CoordPX(nx - ix- ox, ny);
						labelContainer->position=CoordPX(nx, 0.0f);
						labelContainer->dimensions=CoordPerPX(1.0f, 0.0f, 0.0f, ny);
						inputPort->position = CoordPerPX(0.5f, 0.0f,-ix * 0.5f, 0.0f);
						nodeIcon->position=CoordPX(0.0f, iy + 1.0f);
						nodeIcon->dimensions=CoordPerPX(1.0f, 1.0f, 0.0f,-oy - iy- 2.0f);
						inputPortComposite->position=CoordPX(0.0f, iy);
						inputPortComposite->dimensions=CoordPX(0.0f, iy);
						outputPortComposite->position=CoordPerPX(0.0f, 1.0f, 0.0f, -2 * oy);
						outputPortComposite->dimensions=CoordPX(0.0f, oy);
						labelRegion->position=CoordPX(scale*4.0f, 2 * iy + 1.0f);
						labelRegion->fontSize=UnitPX(scale*fontSize);
						nodeIcon->borderScale=scale;
						labelRegion->dimensions=CoordPerPX(0.0f, 1.0f,std::max(scale*(10 + textWidth), 0.5f * nx),-2 * oy- 2 * iy- 2.0f);
					};
			nodeIcon->setAspectRule(AspectRule::FixedHeight);
			nodeIcon->setAspectRatio(1.0f);

			inputPort->setParent(this);
			iconContainer->add(inputPort);
			iconContainer->add(nodeIcon);

			labelRegion->setValue(label);
			labelRegion->setAlignment(HorizontalAlignment::Left, VerticalAlignment::Middle);
			labelRegion->fontSize = UnitPX(fontSize);
			labelRegion->fontType = FontType::Bold;
			labelRegion->backgroundColor = MakeColor(0, 0, 0, 0);
			labelRegion->borderColor = MakeColor(0, 0, 0, 0);
			labelRegion->borderWidth = UnitPX(0.0f);
			labelRegion->onTextEntered = [this](TextField* field) {
				AUnit1D fz=labelRegion->fontSize;
				labelRegion->fontSize=UnitPX(fontSize);
				textWidth = labelRegion->getTextDimensions(AlloyApplicationContext().get()).x;
				labelRegion->fontSize=fz;
				label=field->getValue();
				if(parentFlow->onNodeAction)parentFlow->onNodeAction(this,NodeAction::Modify);
			};
			labelContainer->add(labelRegion);
			inputPortComposite->setOrientation(Orientation::Horizontal, pixel2(2, 0), pixel2(2, 0));
			outputPortComposite->setOrientation(Orientation::Horizontal, pixel2(2, 0), pixel2(2, 0));
			labelContainer->add(inputPortComposite);
			labelContainer->add(outputPortComposite);
			Composite::add(iconContainer);
			Composite::add(labelContainer);
			setRoundCorners(true);
			nodeIcon->backgroundColor = MakeColor(COLOR);
			nodeIcon->borderWidth = borderWidth;
			nodeIcon->setShape(NodeShape::Hexagon);
			forceItem->buoyancy = 0;

		}
		void Source::setup() {
			CompositePtr iconContainer = MakeComposite("Icon Container", CoordPerPX(0.5f, 0.0f, -Node::DIMENSIONS.x * 0.5f, fontSize + 2 * TextField::PADDING),
					CoordPX(Node::DIMENSIONS.x, Node::DIMENSIONS.y - InputPort::DIMENSIONS.y));
			nodeIcon = NodeIconPtr(
					new NodeIcon("Icon", CoordPX(ParentPort::DIMENSIONS.x + 1.0f, 1.0f),
							CoordPerPX(1.0f, 1.0f, -ParentPort::DIMENSIONS.x - ChildPort::DIMENSIONS.x - 2.0f, -OutputPort::DIMENSIONS.y - 2.0f)));
			labelRegion = ModifiableLabelPtr(
					new ModifiableLabel(name, CoordPerPX(0.5f, 0.0f, 0.0f, 0.0f),
							CoordPX(std::max(textWidth + 10.0f, Node::DIMENSIONS.x), fontSize + 2 * TextField::PADDING)));
			nodeIcon->setAspectRatio(1.0f);
			nodeIcon->setAspectRule(AspectRule::FixedHeight);
			outputPort = MakeOutputPort("Output");
			outputPort->setParent(this);
			parentPort = MakeParentPort("Parent");
			childPort = MakeChildPort("Child");
			outputPort->position = CoordPerPX(0.5f, 1.0f, -OutputPort::DIMENSIONS.x * 0.5f, -OutputPort::DIMENSIONS.y);
			parentPort->position = CoordPerPX(0.0f, 0.5f, 0.0f, -OutputPort::DIMENSIONS.y);
			childPort->position = CoordPerPX(1.0f, 0.5f, -ChildPort::DIMENSIONS.x, -OutputPort::DIMENSIONS.y);

			resizeFunc = [this,iconContainer](float scale) {
				const float nx=scale*Node::DIMENSIONS.x;
				const float ny=scale*Node::DIMENSIONS.y;
				//const float ix=scale*InputPort::DIMENSIONS.x;
					const float iy=scale*InputPort::DIMENSIONS.y;
					const float ox=scale*OutputPort::DIMENSIONS.x;
					const float oy=scale*OutputPort::DIMENSIONS.y;
					const float cx=scale*ChildPort::DIMENSIONS.x;
					//const float cy=scale*ChildPort::DIMENSIONS.y;
					const float px=scale*ParentPort::DIMENSIONS.x;
					//const float py=scale*ParentPort::DIMENSIONS.y;
					this->position=CoordPX(0.0f,0.0f);
					this->dimensions = CoordPX(std::max(scale*(textWidth + 10.0f), nx),ny + oy + 2);
					iconContainer->position=CoordPerPX(0.5f, 0.0f, -nx * 0.5f,scale*fontSize + 2 * TextField::PADDING);
					iconContainer->dimensions=CoordPX(nx,ny - iy);
					outputPort->position = CoordPerPX(0.5f, 1.0f,
							-ox * 0.5f, -oy);
					parentPort->position = CoordPerPX(0.0f, 0.5f, 0.0f,-oy);
					childPort->position = CoordPerPX(1.0f, 0.5f, -cx,-oy);
					nodeIcon->position=CoordPX(px + 1.0f, 1.0f);
					nodeIcon->dimensions=CoordPerPX(1.0f, 1.0f,-px - cx- 2.0f, -oy - 2.0f);
					labelRegion->fontSize=UnitPX(scale*fontSize);
					nodeIcon->borderScale=scale;
					labelRegion->position=CoordPerPX(0.5f, 0.0f, 0.0f, 0.0f);
					labelRegion->dimensions=CoordPX(std::max(scale*(textWidth + 10.0f), nx),
							scale*fontSize + 2 * TextField::PADDING);
				};
			childPort->setParent(this);
			parentPort->setParent(this);
			iconContainer->add(nodeIcon);
			iconContainer->add(outputPort);
			iconContainer->add(parentPort);
			iconContainer->add(childPort);
			NVGcontext* nvg = AlloyApplicationContext()->nvgContext;
			nvgFontSize(nvg, fontSize);
			nvgFontFaceId(nvg, AlloyApplicationContext()->getFont(FontType::Bold)->handle);
			textWidth = nvgTextBounds(nvg, 0, 0, label.c_str(), nullptr, nullptr);

			labelRegion->setValue(label);
			labelRegion->setOrigin(Origin::TopCenter);
			labelRegion->setAlignment(HorizontalAlignment::Center, VerticalAlignment::Middle);
			labelRegion->fontSize = UnitPX(fontSize);
			labelRegion->fontType = FontType::Bold;
			labelRegion->backgroundColor = MakeColor(0, 0, 0, 0);
			labelRegion->borderColor = MakeColor(0, 0, 0, 0);
			labelRegion->borderWidth = UnitPX(0.0f);
			labelRegion->onTextEntered = [this](TextField* field) {
				AUnit1D fz=labelRegion->fontSize;
				labelRegion->fontSize=UnitPX(fontSize);
				textWidth = labelRegion->getTextDimensions(AlloyApplicationContext().get()).x;
				labelRegion->fontSize=fz;
				label=field->getValue();
				if(parentFlow->onNodeAction)parentFlow->onNodeAction(this,NodeAction::Modify);
			};
			Composite::add(labelRegion);
			Composite::add(iconContainer);
			setRoundCorners(true);
			nodeIcon->backgroundColor = MakeColor(COLOR);
			nodeIcon->borderWidth = borderWidth;
			forceItem->buoyancy = -1;

		}
		void Destination::setup() {
			CompositePtr iconContainer = MakeComposite("Icon Container", CoordPerPX(0.5f, 0.0f, -Node::DIMENSIONS.x * 0.5f, 0.0f),
					CoordPX(Node::DIMENSIONS.x, Node::DIMENSIONS.y - InputPort::DIMENSIONS.y));
			nodeIcon = NodeIconPtr(
					new NodeIcon("Icon", CoordPX(0.5f * ParentPort::DIMENSIONS.x, InputPort::DIMENSIONS.y + 1.0f),
							CoordPerPX(1.0f, 1.0f, -ParentPort::DIMENSIONS.x, -OutputPort::DIMENSIONS.y - 2.0f)));
			inputPort = MakeInputPort("Input");
			inputPort->position = CoordPerPX(0.5f, 0.0f, -InputPort::DIMENSIONS.x * 0.5f, 0.0f);
			inputPort->setParent(this);
			iconContainer->add(nodeIcon);
			iconContainer->add(inputPort);
			NVGcontext* nvg = AlloyApplicationContext()->nvgContext;
			nvgFontSize(nvg, fontSize);
			nvgFontFaceId(nvg, AlloyApplicationContext()->getFont(FontType::Bold)->handle);
			textWidth = nvgTextBounds(nvg, 0, 0, label.c_str(), nullptr, nullptr);
			labelRegion = ModifiableLabelPtr(
					new ModifiableLabel(name, CoordPerPX(0.5f, 0.0f, 0.0f, Node::DIMENSIONS.y - InputPort::DIMENSIONS.y),
							CoordPX(std::max(textWidth + 10.0f, Node::DIMENSIONS.x), fontSize + 2 * TextField::PADDING)));

			resizeFunc = [this,iconContainer](float scale) {
				const float nx=scale*Node::DIMENSIONS.x;
				const float ny=scale*Node::DIMENSIONS.y;
				const float ix=scale*InputPort::DIMENSIONS.x;
				const float iy=scale*InputPort::DIMENSIONS.y;
				const float oy=scale*OutputPort::DIMENSIONS.y;
				const float px=scale*ParentPort::DIMENSIONS.x;
				this->position=CoordPX(0.0f,0.0f);
				this->dimensions = CoordPX(std::max(scale*(textWidth + 10.0f), nx), ny - iy + scale*fontSize + TextField::PADDING * 2);
				iconContainer->position=CoordPerPX(0.5f, 0.0f, -nx * 0.5f, 0.0f),
				iconContainer->dimensions=CoordPX(nx,ny - iy);
				nodeIcon->position=CoordPX(0.5f * px,iy + 1.0f);
				inputPort->position = CoordPerPX(0.5f, 0.0f,-ix * 0.5f, 0.0f);
				nodeIcon->dimensions=CoordPerPX(1.0f, 1.0f, -px,-oy - 2.0f);
				labelRegion->position=CoordPerPX(0.5f, 0.0f, 0.0f,ny - iy);
				labelRegion->dimensions=CoordPX(std::max(scale*(textWidth + 10.0f), nx),scale*fontSize + 2 * TextField::PADDING);
				labelRegion->fontSize=UnitPX(scale*fontSize);
				nodeIcon->borderScale=scale;
			};
			labelRegion->setValue(label);
			labelRegion->setOrigin(Origin::TopCenter);
			labelRegion->setAlignment(HorizontalAlignment::Center, VerticalAlignment::Middle);
			labelRegion->fontSize = UnitPX(fontSize);
			labelRegion->fontType = FontType::Bold;
			labelRegion->backgroundColor = MakeColor(0, 0, 0, 0);
			labelRegion->borderColor = MakeColor(0, 0, 0, 0);
			labelRegion->borderWidth = UnitPX(0.0f);
			labelRegion->onTextEntered = [this](TextField* field) {
				AUnit1D fz=labelRegion->fontSize;
				labelRegion->fontSize=UnitPX(fontSize);
				textWidth = labelRegion->getTextDimensions(AlloyApplicationContext().get()).x;
				labelRegion->fontSize=fz;
				label=field->getValue();
				if(parentFlow->onNodeAction)parentFlow->onNodeAction(this,NodeAction::Modify);
			};
			Composite::add(iconContainer);
			Composite::add(labelRegion);
			setRoundCorners(true);
			nodeIcon->backgroundColor = MakeColor(COLOR);
			nodeIcon->setShape(NodeShape::Triangle);
			nodeIcon->borderWidth = borderWidth;
			forceItem->buoyancy = 1;

		}
		bool Node::isMouseOver() const {
			if (parent != nullptr)
				return parentFlow->isMouseOverNode(this);
			return false;
		}
		ForceItemPtr& Node::getForceItem() {
			return forceItem;
		}
		void DataFlow::setCurrentPort(Port* currentPort) {
			this->currentPort = currentPort;
		}
		Connection::~Connection() {
			if (source.get() != nullptr)
				source->disconnect(this);
			if (destination.get() != nullptr)
				destination->disconnect(this);
		}
		std::string Port::getReferenceId() const {
			return (parent!=nullptr)?(parent->getId()+"::"+id):id;
		}
		bool Port::disconnect(Connection* connection) {
			bool ret = false;
			for (auto iter = connections.begin(); iter != connections.end(); iter++) {
				if (iter->get() == connection) {
					connections.erase(iter);
					ret = true;
					if (connections.size() > 0) {
						iter--;
					} else
						break;
				}
			}
			return ret;
		}
		std::shared_ptr<Group> DataFlow::groupNodes(const std::vector<std::shared_ptr<Node>>& nodes) {
			std::set<Node*> activeList;
			for (NodePtr n : nodes) {
				activeList.insert(n.get());
			}
			GroupPtr group = MakeGroupNode("Group");
			group->nodes = nodes;
			for (ConnectionPtr connection : data->connections) {
				connection->setSelected(false); //Do not delete connections on group
				Node* src = connection->source->getNode();
				Node* dest = connection->destination->getNode();
				if ( activeList.find(src)!=activeList.end() &&
					 activeList.find(dest) != activeList.end() &&
					 !src->hasExternalPorts() &&
					 !dest->hasExternalPorts()) {
					//Add connections between source / dest to new group
					group->connections.push_back(connection);
				}
			}
			float2 center(0.0f);
			std::list<ConnectionPtr> connectionList;
			std::list<RelationshipPtr> relationshipList;
			for (NodePtr node : nodes) {
					center += node->getLocation();
					for (InputPortPtr port : node->getInputPorts()) {
						if (port->isConnected()) {
							InputPortPtr newPort = MakeInputPort(port->name);
							bool outside = false;
							for (ConnectionPtr connection : port->getConnections()) {
								if (activeList.find(connection->source->getNode()) == activeList.end()) {
									outside = true;
									connectionList.push_back(MakeConnection(connection->source, newPort));
									connection->source->disconnect(connection);
								}
							}
							if (outside) {
								for (auto iter = port->getConnections().begin(); iter != port->getConnections().end(); iter++) {
									ConnectionPtr connection = *iter;
									if (activeList.find(connection->source->getNode()) == activeList.end()) {
										port->getConnections().erase(iter);
										if (port->getConnections().size() > 0) {
											iter--;
										}
										else {
											break;
										}
									}
								}
								port->setProxyIn(newPort);
								newPort->setProxyOut(port);
								newPort->copyContentsFrom(port);
								group->add(newPort);
							}
						}
					}
					for (OutputPortPtr port : node->getOutputPorts()) {
						if (port->isConnected()) {
							OutputPortPtr newPort = MakeOutputPort(port->name);
							bool outside = false;
							for (ConnectionPtr connection : port->getConnections()) {
								Node* dest = connection->destination->getNode();
								if (activeList.find(connection->destination->getNode()) == activeList.end() && !dest->hasExternalInput()) {
									outside = true;
									connectionList.push_back(MakeConnection(newPort, connection->destination));
									connection->destination->disconnect(connection);
								}
							}
							if (outside) {
								for (auto iter = port->getConnections().begin(); iter != port->getConnections().end(); iter++) {
									ConnectionPtr connection = *iter;
									Node* dest = connection->destination->getNode();
									if (activeList.find(connection->destination->getNode()) == activeList.end() && !dest->hasExternalInput()) {
										port->getConnections().erase(iter);
										if (port->getConnections().size() > 0) {
											iter--;
										}
										else {
											break;
										}
									}
								}
								port->setProxyIn(newPort);
								newPort->setProxyOut(port);
								newPort->copyContentsFrom(port);
								group->add(newPort);
							}
						}
					}
					{
						InputPortPtr port = node->getInputPort();
						if (port.get() != nullptr && port->isConnected()) {
							InputPortPtr newPort = MakeInputPort(port->name);
							bool outside = false;
							for (ConnectionPtr connection : port->getConnections()) {
								Node* src = connection->source->getNode();
								if (activeList.find(connection->source->getNode()) == activeList.end() && !src->hasExternalOutput()) {
									outside = true;
									connectionList.push_back(MakeConnection(connection->source, newPort));
									connection->source->disconnect(connection.get());
									connection->destination->disconnect(connection.get());
								}
							}
							if (outside) {
								port->setProxyIn(newPort);
								newPort->setProxyOut(port);
								newPort->copyContentsFrom(port);
								group->add(newPort);
							}
						}
					}
					{
						OutputPortPtr port = node->getOutputPort();
						if (port.get() != nullptr && port->isConnected()) {
							OutputPortPtr newPort = MakeOutputPort(port->name);
							bool outside = false;
							for (ConnectionPtr connection : port->getConnections()) {
								Node* src = connection->source->getNode();
								if (activeList.find(connection->source->getNode()) == activeList.end() && !src->hasExternalOutput()) {
									outside = true;
									connectionList.push_back(MakeConnection(newPort, connection->destination));
									connection->source->disconnect(connection.get());
									connection->destination->disconnect(connection.get());
								}
							}
							if (outside) {
								port->setProxyIn(newPort);
								newPort->setProxyOut(port);
								newPort->copyContentsFrom(port);
								group->add(newPort);
							}
						}
					}
			}
			for (RelationshipPtr relationship : data->relationships) {
				if (activeList.find(relationship->object.get()) != activeList.end() ||
					activeList.find(relationship->subject.get()) != activeList.end()) {
					group->relationships.push_back(relationship);
				}
			}
			if (group->nodes.size() == 0)
				return false;
			center /= (float)group->nodes.size();
			group->setLocation(center);
			addNodeInternal(group);
			if (onNodeAction) {
				onNodeAction(group.get(), NodeAction::Add);
			}
			deleteSelected();
			for (ConnectionPtr connection : group->connections) {
				connection->source->connect(connection);
				connection->destination->connect(connection);
			}
			for (ConnectionPtr connection : connectionList) {
				add(connection);
				connection->update();
			}
			for (RelationshipPtr relationship : relationshipList) {
				add(relationship);
				relationship->update();
			}
			for (NodePtr node : group->nodes) {
				node->setSelected(false);
			}
			for (ConnectionPtr connection : group->connections) {
				connection->setSelected(false);
			}

			group->setSelected(true);
			if (onNodeAction) {
				onNodeAction(group.get(), NodeAction::Focus);
				onNodeAction(group.get(), NodeAction::Select);
			}
			//std::cout<<"------- Group Selected -------"<<std::endl;
			//std::cout<<*group<<std::endl;
			return group;
		}
		std::vector<NodePtr> DataFlow::ungroupNodes(const std::vector<std::shared_ptr<Node>>& deleteList) {
			for (ConnectionPtr connection : data->connections) {
				connection->setSelected(false);	//Do not delete connections on ungroup
			}
			std::vector<NodePtr> nodeList;
			for (NodePtr n : deleteList) {
				GroupPtr group = std::dynamic_pointer_cast<Group>(n);
				if (group.get() == nullptr)continue;
				group->setSelected(true);
				std::list<ConnectionPtr> connectionList;
				std::list<RelationshipPtr> relationshipList;
				float2 center(0.0f);
				if (group->nodes.size() > 0) {
					for (NodePtr node : group->nodes) {
						center += node->getLocation();
					}
					center /= (float)group->nodes.size();
					center -= group->getLocation();
					for (NodePtr node : group->nodes) {
						node->setLocation(node->getLocation() - center);
					}
					nodeList.insert(nodeList.end(), group->nodes.begin(), group->nodes.end());
				}
				relationshipList.insert(relationshipList.end(), group->relationships.begin(), group->relationships.end());
				connectionList.insert(connectionList.end(), group->connections.begin(), group->connections.end());
				for (ConnectionPtr connection : group->connections) {
					connection->source->connect(connection);
					connection->destination->connect(connection);
				}
				for (InputPortPtr port : group->getInputPorts()) {
					PortPtr proxy = port->getProxyOut();
					if (proxy.get() != nullptr) {
						proxy->setProxyIn();
						for (ConnectionPtr connection : port->getConnections()) {
							connectionList.push_back(MakeConnection(connection->source, proxy));
							connection->source->disconnect(connection);
						}
					}
					port->setProxyOut();
					port->getConnections().clear();
				}
				for (OutputPortPtr port : group->getOutputPorts()) {
					PortPtr proxy = port->getProxyOut();
					if (proxy.get() != nullptr) {
						proxy->setProxyIn();
						for (ConnectionPtr connection : port->getConnections()) {
							connectionList.push_back(MakeConnection(proxy, connection->destination));
							connection->destination->disconnect(connection);
						}
					}
					port->setProxyOut();
					port->getConnections().clear();
				}
				deleteSelected();
				for (NodePtr node : group->nodes) {
					node->setSelected(false);
					addNodeInternal(node);
					if (onNodeAction) {
						onNodeAction(node.get(), NodeAction::Add);
					}
				}
				for (ConnectionPtr connection : connectionList) {
					add(connection);
				}
				for (RelationshipPtr connection : relationshipList) {
					add(connection);
				}
			}
			for (NodePtr node : nodeList) {
				node->setSelected(true);
				if (onNodeAction) {
					onNodeAction(node.get(), NodeAction::Select);
				}
			}
			if (nodeList.size() == 0) {
				if (onNodeAction) {
					onNodeAction(nullptr, NodeAction::Select);
				}
			}
			return nodeList;
		}

		bool DataFlow::groupSelected() {
			std::vector<NodePtr> nodes;
			for (NodePtr node : data->nodes) {
				if (node->isSelected()) {
					if (node->hasExternalPorts()) {
						return false;
					}
					nodes.push_back(node);
				}
			}
			return (groupNodes(nodes).get()!=nullptr);
		}
		bool DataFlow::ungroupSelected() {
			std::vector<NodePtr> deleteList;
			for (NodePtr node : data->nodes) {
				if (node->isSelected() && node->getType() == NodeType::Group) {
					if (!node->hasExternalPorts()) {
						GroupPtr group = std::dynamic_pointer_cast<Group>(node);
						deleteList.push_back(group);
					} else {
						return false;
					}
				}
				node->setSelected(false);	//Do not delete things that are not groups.
			}
			ungroupNodes(deleteList);
			return true;
		}
		void DataFlow::clear() {
			forceSim->stop();
			Composite::clear();
			forceSim->clear();
			router.update();
			currentPort = nullptr;
			connectingPort = nullptr;
			AlloyApplicationContext()->requestPack();
		}
		bool DataFlow::remove(Connection* selectedConnection) {
			std::list<SpringItemPtr> deleteSpringList;
			deleteSpringList.push_back(selectedConnection->getSpringItem());
			if (data->remove(selectedConnection)) {
				if (onConnectionAction)
					onConnectionAction(selectedConnection, ConnectionAction::Delete);
				currentPort = nullptr;
				connectingPort = nullptr;
				forceSim->erase(deleteSpringList);
				return true;
			} else {
				return false;
			}
		}
		bool DataFlow::remove(Relationship* selectedConnection) {
			std::list<SpringItemPtr> deleteSpringList;
			deleteSpringList.push_back(selectedConnection->getSpringItem());
			if (data->remove(selectedConnection)) {
				currentPort = nullptr;
				connectingPort = nullptr;
				forceSim->erase(deleteSpringList);
				return true;
			}
			else {
				return false;
			}
		}

		std::string DataFlow::toString(std::string indent) const {
			std::stringstream ss;
			ss << indent << "DataFlow [" << getName() <<"]" << std::endl;
			indent += "  ";
			for (NodePtr node : getNodes()) {
				if (node->getType() != NodeType::Group) {
					ss << indent << "Node [" << node->getLabel() << "::" << node->getId() << "] " << (node->hasContents() ? "has contents" : "no contents")
							<< " Parent:" << ((node->parent != nullptr) ? node->parent->getName() : "null") << std::endl;
				}
			}
			for (ConnectionPtr connection : getConnections()) {
				ss << indent << "Connection " << *connection << std::endl;
			}
			for (NodePtr node : getNodes()) {
				if (node->getType() != NodeType::Group) {
					for (InputPortPtr port : node->inputPorts) {
						if (port->hasProxyIn() || port->hasProxyOut()) {
							ss << indent << "Proxy Input " << *port << std::endl;
						}
					}
					for (OutputPortPtr port : node->outputPorts) {
						if (port->hasProxyIn() || port->hasProxyOut()) {
							ss << indent << "Proxy Output " << *port << std::endl;
						}
					}
				}
			}
			{
				for (InputPortPtr port : data->inputPorts) {
					if (port->hasProxyIn() || port->hasProxyOut()) {
						ss << indent << "Proxy Input " << *port << std::endl;
					}
				}
				for (OutputPortPtr port : data->outputPorts) {
					if (port->hasProxyIn() || port->hasProxyOut()) {
						ss << indent << "Proxy Output " << *port << std::endl;
					}
				}
			}
			for (NodePtr node : getNodes()) {
				if (node->getType() == NodeType::Group) {
					GroupPtr group = std::dynamic_pointer_cast<Group>(node);
					ss << group->toString(indent);

				}
			}
			return ss.str();
		}
		std::string Group::toString(std::string indent) const {
			std::stringstream ss;
			ss << indent << "Group [" << getLabel() << "::" << getId()<< "] " << (hasContents() ? "has contents" : "no contents") << std::endl;
			indent += "  ";
			for (NodePtr node : getNodes()) {
				if (node->getType() != NodeType::Group) {
					ss << indent << "Node [" << node->getLabel() << "::" << node->getId() << "] " << (node->hasContents() ? "has contents" : "no contents")
							<< " Parent:" << ((node->parent != nullptr) ? node->parent->getName() : "null") << std::endl;
				}
			}
			for (ConnectionPtr connection : getConnections()) {
				ss << indent << "Connection " << *connection << std::endl;
			}
			for (InputPortPtr port : inputPorts) {
				if (port->hasProxyIn() || port->hasProxyOut()) {
					ss << indent << "Proxy Input " << *port << std::endl;
				}
			}
			for (OutputPortPtr port : outputPorts) {
				if (port->hasProxyIn() || port->hasProxyOut()) {
					ss << indent << "Proxy Output " << *port << std::endl;
				}
			}
			for (NodePtr node : getNodes()) {
				if (node->getType() != NodeType::Group) {
					for (InputPortPtr port : node->getInputPorts()) {
						if (port->hasProxyIn() || port->hasProxyOut()) {
							ss << indent << "Proxy Input " << *port << std::endl;
						}
					}
					for (OutputPortPtr port : node->getOutputPorts()) {
						if (port->hasProxyIn() || port->hasProxyOut()) {
							ss << indent << "Proxy Output " << *port << std::endl;
						}
					}
				}
			}
			for (NodePtr node : getNodes()) {
				if (node->getType() == NodeType::Group) {
					GroupPtr group = std::dynamic_pointer_cast<Group>(node);
					ss << group->toString(indent);
				}
			}
			return ss.str();
		}
		bool DataFlow::copySelected() {
			GroupPtr group = std::dynamic_pointer_cast<Group>(data->clone(true));
			if (group->nodes.size() == 0)return false;
			//std::cout << "Copy\n" << group->toString() << std::endl;
			clipboard = group;
			if(onNodeAction){
				onNodeAction(clipboard.get(),NodeAction::Copy);
			}
			return true;
		}
		bool DataFlow::paste(pixel2 offset) {
			if (clipboard.get() != nullptr) {
				GroupPtr group = std::dynamic_pointer_cast<Group>(clipboard->clone());
				pixel2 center=group->getLocation();
				if(distance(offset,center)<0.5f*Node::DIMENSIONS.x){
					offset=center+Node::DIMENSIONS*0.5f;
				}
				//std::cout << "Paste\n" <<group->toString()<< std::endl;
				for (NodePtr node : data->nodes) {
					node->setSelected(false);
				}
				for (NodePtr node : group->getNodes()) {
					node->setSelected(true);
					node->setLocation(node->getLocation() + offset - center);
					addNodeInternal(node);
					node->setLabel(node->getLabel() + " Copy", false);
				}
				for (ConnectionPtr con : group->getConnections()) {
					add(con);
				}
				for (RelationshipPtr con : group->getRelationships()) {
					add(con);
				}
				AlloyApplicationContext()->requestPack();
				if(onNodeAction){
					onNodeAction(group.get(),NodeAction::Paste);
				}
				return true;
			} else {
				return false;
			}
		}
		bool DataFlow::cutSelected() {
			if (copySelected()) {
				return deleteSelected(true);
			} else {
				return false;
			}
		}
		void DataFlow::setScale(float f, pixel2 pos) {
			pixel2 dragOffset = this->extents.position;
			box2f bounds = getBounds();
			pixel2 offset = bounds.position;
			float2 pw = (pos - offset - dragOffset) / scale;
			scale = f;
			this->extents.position = pos - offset - scale * pw;
			router.setBorderSpacing(10.0f * scale);
		}
		bool DataFlow::deleteSelected(bool externalCheck) {
			std::list<ForceItemPtr> deleteForceList;
			std::list<NodePtr> deleteNodeList;
			for (RegionPtr child : children) {
				Node* node = dynamic_cast<Node*>(child.get());
				if (node) {
					if (externalCheck) {
						if (node->isSelected() && node->hasExternalPorts()) {
							return false;
						}
					}
				}
			}
			{
				std::vector<RegionPtr> tmpList;
				router.nodes.clear();
				data->nodes.clear();
				for (RegionPtr child : children) {
					Node* node = dynamic_cast<Node*>(child.get());
					if (node) {
						bool notExternalized = (!externalCheck || (externalCheck && !node->hasExternalPorts()));
						if (node->isSelected() && notExternalized) {
							deleteForceList.push_back(node->forceItem);
							deleteNodeList.push_back(std::dynamic_pointer_cast<Node>(child));
							node->parent = nullptr;

						} else {
							NodePtr nodePtr = std::dynamic_pointer_cast<Node>(child);
							router.nodes.push_back(std::dynamic_pointer_cast<AvoidanceNode>(nodePtr));
							data->nodes.push_back(nodePtr);
							tmpList.push_back(child);
							if (!notExternalized) {
								node->setSelected(false);
							}
						}
					} else {
						tmpList.push_back(child);
					}
				}
				router.update();
				children = tmpList;
			}
			std::list<SpringItemPtr> deleteSpringList;
			{
				std::vector<ConnectionPtr> tmpList;
				for (ConnectionPtr connection : data->connections) {
					if (connection->source->getNode()->isSelected() || connection->destination->getNode()->isSelected() || connection->selected) {
						if (onConnectionAction) {
							onConnectionAction(connection.get(), ConnectionAction::Delete);
						}
						if (connection->selected) {
							deleteSpringList.push_back(connection->getSpringItem());
						}
					} else {
						tmpList.push_back(connection);
					}
				}
				routingLock.lock();
				data->connections = tmpList;
				routingLock.unlock();
			}

			{
				std::vector<RelationshipPtr> tmpList;
				for (RelationshipPtr relationship : data->relationships) {
					if (!relationship->object->isSelected() && !relationship->subject->isSelected()) {
						tmpList.push_back(relationship);
					}
				}
				data->relationships = tmpList;
			}
			if (onNodeAction){
				for(NodePtr node:deleteNodeList){
					onNodeAction(node.get(), NodeAction::Delete);
				}
			}
			currentPort = nullptr;
			connectingPort = nullptr;
			forceSim->erase(deleteForceList);
			forceSim->erase(deleteSpringList);
			return true;
		}
		bool DataFlow::onEventHandler(AlloyContext* context, const InputEvent& e) {
			if (!isVisible() || context->getGlassPane()->isVisible()) {
				stop();
				mouseOverNode = nullptr;
				mouseDragNode = nullptr;
				mouseDownInRegion = false;
				return false;
			}
			bool mouseOver = context->isMouseOver(this, true);
			if (e.type == InputType::Scroll && mouseOver && !context->isMouseDown()) {
				setScale(clamp(scale * (1.0f + e.scroll.y * 0.1f), 0.1f, 10.0f), e.cursor);
				context->requestPack();
				return true;
			}
			if (connectingPort != nullptr && e.type == InputType::MouseButton && e.isUp()) {
				if (currentPort != nullptr && currentPort != connectingPort && context->isMouseOver(currentPort)) {
					PortPtr source = connectingPort->getReference();
					PortPtr target = currentPort->getReference();
					ConnectionPtr last;
					if (source->getType() == PortType::Output && target->getType() == PortType::Input) {
						if (onValidateConnection) {
							if (onValidateConnection(source.get(), target.get())) {
								add(last = MakeConnection(source, target));
							}
						} else {
							add(last = MakeConnection(source, target));
						}
					} else if (source->getType() == PortType::Input && target->getType() == PortType::Output) {
						if (onValidateConnection) {
							if (onValidateConnection(target.get(), source.get())) {
								add(last = MakeConnection(target, source));
							}
						} else {
							add(last = MakeConnection(target, source));
						}
					} else if (source->getType() == PortType::Parent && target->getType() == PortType::Child) {
						if (onValidateConnection) {
							if (onValidateConnection(target.get(), source.get())) {
								add(last = MakeConnection(target, source));
							}
						} else {
							add(last = MakeConnection(target, source));
						}
					} else if (source->getType() == PortType::Child && target->getType() == PortType::Parent) {
						if (onValidateConnection) {
							if (onValidateConnection(source.get(), target.get())) {
								add(last = MakeConnection(source, target));
							}
						} else {
							add(last = MakeConnection(source, target));
						}
					}
				}
				connectingPort = nullptr;
			}

			if (Composite::onEventHandler(context, e))
				return true;

			if (mouseOverNode != nullptr) {
				forceSim->setSelected(mouseOverNode->forceItem.get());
				mouseOverNode->forceItem->velocity = float2(0.0f);
				mouseOverNode->forceItem->plocation = mouseOverNode->forceItem->location;
				if (e.type == InputType::MouseButton && e.isDown() && e.button == GLFW_MOUSE_BUTTON_LEFT) {
					if (e.clicks == 2) {
						if (!mouseOverNode->isSelected()) {
							mouseOverNode->setSelected(true);
							if (onNodeAction)
								onNodeAction(mouseOverNode, NodeAction::Select);
						} else {
							if (onNodeAction)
								onNodeAction(mouseOverNode, NodeAction::Focus);
						}
						if (onNodeAction)
							onNodeAction(mouseOverNode, NodeAction::DoubleClick);

						return true;
					} else {
						if (onNodeAction)
							onNodeAction(mouseOverNode, NodeAction::Focus);
					}
				}
			}
			if (mouseDragNode != nullptr) {
				if (draggingNode && e.type == InputType::Cursor) {
					for (std::pair<Node*, pixel2> pr : dragList) {
						pr.first->setDragOffset(e.cursor, pr.second);
					}
					dragAction = true;
					context->requestPack();
				} else if (e.type == InputType::MouseButton && e.isUp()) {
					context->requestPack();
					dragList.clear();
					mouseDragNode = nullptr;
					dragAction = false;
					draggingNode = false;
				}
			}
			if (!draggingNode && mouseSelectedNode != nullptr) {
				if (e.type == InputType::MouseButton && e.button == GLFW_MOUSE_BUTTON_LEFT && e.isDown()) {
					mouseDragNode = mouseSelectedNode;
					if (context->isShiftDown()) {
						mouseDragNode->setSelected(!mouseDragNode->isSelected());
					}
					putLast(mouseDragNode);
					dragList.clear();
					if (mouseDragNode->isSelected()) {
						for (RegionPtr child : children) {
							Node* node = dynamic_cast<Node*>(child.get());
							if (node && node->isSelected()) {
								dragList.push_back(std::pair<Node*, pixel2>(node, e.cursor - node->getBoundsPosition()));
							}
						}
					} else {
						dragList.push_back(std::pair<Node*, pixel2>(mouseDragNode, e.cursor - mouseDragNode->getBoundsPosition()));
					}
					draggingNode = true;
					dragAction = false;
				}
			}
			if (e.type == InputType::Cursor || e.type == InputType::MouseButton) {
				if (context->isMouseDrag() && e.button == GLFW_MOUSE_BUTTON_LEFT) {
					if (dragBox.dimensions.x * dragBox.dimensions.y > 0
							|| (connectingPort == nullptr && mouseOverNode == nullptr && mouseDragNode == nullptr)) {
						if (mouseDownInRegion) {
							float2 cursorDown = context->getCursorDownPosition();
							box2px box = getBounds();
							float2 stPt = aly::min(cursorDown, e.cursor);
							float2 endPt = aly::max(cursorDown, e.cursor);
							dragBox.position = stPt;
							dragBox.dimensions = endPt - stPt;
							dragBox.intersect(box);
						}
					}
				}
			}
			if (e.type == InputType::Cursor) {
				if (!mouseOver) {
					forceSim->setSelected(nullptr);
				}
			}
			if (e.type == InputType::MouseButton) {
				if (e.isDown()) {
					if (e.button == GLFW_MOUSE_BUTTON_LEFT) {
						if (context->isMouseContainedIn(this)) {
							mouseDownInRegion = true;
						}
						if (selectedConnection) {
							if (!context->isShiftDown()) {
								for (ConnectionPtr connection : data->connections) {
									connection->selected = false;
								}
							}
							selectedConnection->selected = true;
							if (onConnectionAction)
								onConnectionAction(selectedConnection, ConnectionAction::Select);

						}
					}
					if (!context->isControlDown()) {
						forceSim->stop();
					}

				} else if (e.isUp()) {
					mouseDownInRegion = false;
					if (e.button == GLFW_MOUSE_BUTTON_RIGHT && !dragAction && draggingGraph) {

						bool deselect = false;
						for (ConnectionPtr connection : data->connections) {
							if (connection->selected) {
								deselect = true;
							}
							connection->selected = false;
						}
						if (deselect && onConnectionAction) {
							onConnectionAction(nullptr, ConnectionAction::Select);
						}
						deselect = false;
						for (RegionPtr child : children) {
							Node* node = dynamic_cast<Node*>(child.get());
							if (node) {
								if (node->isSelected()) {
									deselect = true;
								}
								node->setSelected(false);
							}
						}
						if (deselect && onNodeAction) {
							onNodeAction(nullptr, NodeAction::Select);
						}

					}
					start();
				}
			}
			if (e.type == InputType::MouseButton && e.isUp()) {
				if (draggingGraph) {
					draggingGraph = false;
					dragAction = false;
				} else if (draggingNode) {
					dragAction = false;
					draggingNode = false;
				} else if (dragBox.dimensions.x * dragBox.dimensions.y > 0) {
					pixel2 offset = getDrawOffset() + getBoundsPosition();
					Node* node = nullptr;
					Node* lastNode = nullptr;
					for (RegionPtr child : children) {
						node = dynamic_cast<Node*>(child.get());
						if (node) {
							if (dragBox.contains(node->getForceItem()->location * scale + offset)) {
								node->setSelected(true);
								if (onNodeAction)
									onNodeAction(node, NodeAction::Select);
								lastNode = node;
							}
						}
					}
					if (lastNode && onNodeAction) {
						onNodeAction(lastNode, NodeAction::Focus);
					}
					dragBox = box2px(pixel2(0.0f), pixel2(0.0f));
				}
			}

			if (e.type == InputType::MouseButton && e.isDown() && e.button == GLFW_MOUSE_BUTTON_RIGHT && context->isMouseOver(this, true)) {
				currentDrawOffset = this->extents.position;
				cursorDownLocation = e.cursor;
				draggingGraph = true;
				dragAction = false;
			}
			if (e.type == InputType::Cursor && draggingGraph) {
				this->extents.position = currentDrawOffset + e.cursor - cursorDownLocation;
				if (lengthL1(e.cursor - cursorDownLocation) > 5) {
					dragAction = true;
				}
			}
			if (e.isDown() && e.type == InputType::Key && mouseOver) {
				switch (e.key) {
				case GLFW_KEY_DELETE:
					if (!deleteSelected(true)) {
						errorMessage->setMessage("Could not delete selected because node(s) have external ports.");
						errorMessage->setVisible(true);
						AlloyApplicationContext()->requestPack();
					}
					return true;
					break;
				case GLFW_KEY_A:
					if (e.isControlDown()) {
						Region* region = context->getMouseFocusObject();
						if (region == nullptr || dynamic_cast<TextField*>(region) == nullptr) {
							for (RegionPtr child : children) {
								Node* node = dynamic_cast<Node*>(child.get());
								if (node) {
									node->setSelected(true);
									if (onNodeAction)
										onNodeAction(node, NodeAction::Select);
								}
							}
							return true;
						}
					}
					break;
				case GLFW_KEY_D:
					if (e.isControlDown()) {
						for (RegionPtr child : children) {
							Node* node = dynamic_cast<Node*>(child.get());
							if (node) {
								node->setSelected(false);
							}
						}
						return true;
					}
					break;
				case GLFW_KEY_G:
					if (e.isControlDown()) {
						if (!groupSelected()) {
							errorMessage->setMessage("Could not group selected because node(s) have external ports.");
							errorMessage->setVisible(true);
							AlloyApplicationContext()->requestPack();
						}
					}
					break;
				case GLFW_KEY_U:
					if (e.isControlDown()) {
						if (!ungroupSelected()) {
							errorMessage->setMessage("Could not ungroup selected because node(s) have external ports.");
							errorMessage->setVisible(true);
							AlloyApplicationContext()->requestPack();
						}
					}
					break;
				case GLFW_KEY_X:
					if (e.isControlDown()) {
						if (!cutSelected()) {
							errorMessage->setMessage("Could not cut selected because node(s) have external ports.");
							errorMessage->setVisible(true);
							AlloyApplicationContext()->requestPack();
						}
					}
					break;
				case GLFW_KEY_C:
					if (e.isControlDown()) {
						return copySelected();
					}
					break;
				case GLFW_KEY_V:
					if (e.isControlDown()) {
						pixel2 pos = e.cursor;
						if (!bounds.contains(pos)) {
							pos = bounds.center();
						}
						pixel2 offset = getDrawOffset() + getBoundsPosition();
						return paste((pos - offset) / getScale());
					}
					break;
				case GLFW_KEY_Z:
					if (e.isControlDown()) {
						return undo();
					}
					break;
				case GLFW_KEY_Y:
					if (e.isControlDown()) {
						return redo();
					}
					break;
				case GLFW_KEY_I:
					if (e.isControlDown()) {
						for (RegionPtr child : children) {
							Node* node = dynamic_cast<Node*>(child.get());
							if (node) {
								node->setSelected(!node->isSelected());
							}
						}
						return true;
					}
					break;
				}
			}
			return Composite::onEventHandler(context, e);
		}
		float Connection::distance(const float2& pt) {
			float minD = 1E30f;
			for (int i = 0; i < (int) path.size() - 1; i++) {
				lineseg2f ls(path[i], path[i + 1]);
				minD = std::min(ls.distance(pt), minD);
			}
			return minD;
		}
		void Connection::draw(AlloyContext* context, DataFlow* flow) {
			if (path.size() == 0)
				return;
			const float scale = flow->getScale();
			NVGcontext* nvg = context->nvgContext;
			if (selected) {
				nvgStrokeWidth(nvg, std::max(scale * 6.0f, 1.0f));
				nvgStrokeColor(nvg, context->theme.LIGHTEST);
			} else {
				nvgStrokeWidth(nvg, std::max(scale * 4.0f, 1.0f));
				nvgStrokeColor(nvg, context->theme.LIGHTEST.toDarker(0.8f));
			}

			float2 offset = flow->getDrawOffset();
			nvgLineCap(nvg, NVG_ROUND);
			nvgLineJoin(nvg, NVG_BEVEL);
			nvgBeginPath(nvg);
			float2 pt0 = offset + path.front();
			nvgMoveTo(nvg, pt0.x, pt0.y);
			for (int i = 1; i < (int) path.size() - 1; i++) {
				float2 pt1 = offset + path[i];
				float2 pt2 = offset + path[i + 1];
				float diff = 0.5f
						* std::min(std::max(std::abs(pt1.x - pt2.x), std::abs(pt1.y - pt2.y)), std::max(std::abs(pt1.x - pt0.x), std::abs(pt1.y - pt0.y)));
				if (diff < scale * context->theme.CORNER_RADIUS) {
					nvgLineTo(nvg, pt1.x, pt1.y);
				} else {
					nvgArcTo(nvg, pt1.x, pt1.y, pt2.x, pt2.y, scale * context->theme.CORNER_RADIUS);
				}
				pt0 = pt1;
			}
			pt0 = offset + path.back();
			nvgLineTo(nvg, pt0.x, pt0.y);
			nvgStroke(nvg);
		}
		bool DataFlow::intersects(const lineseg2f& ln) {
			for (box2px box : router.getObstacles()) {
				if (ln.intersects(box))
					return true;
			}
			return false;
		}
		void DataFlow::startConnection(Port* port) {
			connectingPort = port;
		}
		void DataFlow::addRegion(const std::shared_ptr<Node>& node) {
			node->parentFlow = this;
			Composite::add(node);
			routingLock.lock();
			router.add(std::dynamic_pointer_cast<AvoidanceNode>(node));
			routingLock.unlock();
			forceSim->addForceItem(node->getForceItem());
			box2px box = box2px(node->getForceItem()->location - Node::DIMENSIONS * 0.5f, Node::DIMENSIONS);
			graphBounds.merge(box);
			forceSim->setBounds(CoordPX(graphBounds.position), CoordPX(graphBounds.dimensions));
			boxForce->setBounds(graphBounds);
			if (onNodeAction) {
				onNodeAction(node.get(), NodeAction::Add);
			}
		}
		bool DataFlow::undo() {
			return history.undo();
		}
		bool DataFlow::redo() {
			return history.redo();
		}
		void DataFlow::addRegion(const std::shared_ptr<Connection>& connection) {
			SpringItem* spring = new SpringItem(connection->source->getNode()->getForceItem(), connection->destination->getNode()->getForceItem(), -1.0f,
					2 * ForceSimulator::RADIUS);
			spring->gamma = 0.1f;
			spring->visible = false;
			spring->length = distance(spring->item1->location, spring->item2->location);
			connection->getSpringItem().reset(spring);
			routingLock.lock();
			forceSim->addSpringItem(connection->getSpringItem());
			routingLock.unlock();
			if (onConnectionAction) {
				onConnectionAction(connection.get(), ConnectionAction::Add);
			}
		}
		void DataFlow::add(const std::shared_ptr<Region>& region) {
			Composite::add(region);
		}
		std::vector<std::shared_ptr<Connection>>& DataFlow::getConnections() {
			return data->connections;
		}
		std::shared_ptr<Connection> DataFlow::getConnection(Connection* con) {
			return data->getConnection(con);
		}
		Group* DataFlow::getGroup(Connection* con) {
			return data->getGroup(con);
		}
		std::vector<std::shared_ptr<Relationship>>& DataFlow::getRelationships() {
			return data->relationships;
		}
		std::vector<std::shared_ptr<Node>>& DataFlow::getNodes() {
			return data->nodes;
		}
		const std::vector<std::shared_ptr<Connection>>& DataFlow::getConnections() const {
			return data->connections;
		}
		const std::vector<std::shared_ptr<Relationship>>& DataFlow::getRelationships() const {
			return data->relationships;
		}
		const std::vector<std::shared_ptr<Node>>& DataFlow::getNodes() const {
			return data->nodes;
		}
		void DataFlow::add(const std::shared_ptr<Relationship>& relationship) {
			addRelationshipInternal(relationship);
		}
		void DataFlow::addRelationshipInternal(const std::shared_ptr<Relationship>& relationship) {
			relationship->getSpringItem().reset(
					new SpringItem(relationship->object->getForceItem(), relationship->subject->getForceItem(), -1.0f,
							std::max(distance(relationship->object->getForceItem()->location, relationship->subject->getForceItem()->location),
									2 * Node::DIMENSIONS.x)));
			relationship->getSpringItem()->visible = false;
			routingLock.lock();
			forceSim->addSpringItem(relationship->getSpringItem());
			data->relationships.push_back(relationship);
			routingLock.unlock();
		}
		void DataFlow::removeNode(const std::shared_ptr<Node>& node) {
		}
		void DataFlow::addNodeInternal(const std::shared_ptr<Node>& node) {
			node->parentFlow = this;
			Composite::add(node);
			bool contains = false;
			for (auto n : data->nodes) {
				if (n.get() == node.get()) {
					contains = true;
					break;
				}
			}
			if (!contains) {
				data->nodes.push_back(node);
			}
			routingLock.lock();
			router.add(std::dynamic_pointer_cast<AvoidanceNode>(node));
			routingLock.unlock();
			forceSim->addForceItem(node->getForceItem());
			box2px box = box2px(node->getForceItem()->location - Node::DIMENSIONS * 0.5f, Node::DIMENSIONS);
			graphBounds.merge(box);
			forceSim->setBounds(CoordPX(graphBounds.position), CoordPX(graphBounds.dimensions));
			boxForce->setBounds(graphBounds);
		}
		void DataFlow::addNode(const std::shared_ptr<Node>& node) {
			addNodeInternal(node);
			if (onNodeAction) {
				onNodeAction(node.get(), NodeAction::Add);
			}

		}
		void DataFlow::setGroup(const std::shared_ptr<Group>& g) {
			routingLock.lock();
			router.nodes.clear();
			for (auto n : g->nodes) {
				router.nodes.push_back(std::dynamic_pointer_cast<AvoidanceNode>(n));
			}
			router.update();
			routingLock.unlock();
			forceSim->clear();
			std::vector<RegionPtr> tmpList;
			for (RegionPtr child : children) {
				Node* node = dynamic_cast<Node*>(child.get());
				if (node == nullptr) {
					tmpList.push_back(child);
				}
			}
			children = tmpList;
			for (NodePtr node : g->nodes) {
				node->parent = this;
				forceSim->addForceItem(node->getForceItem());
				children.push_back(node);
			}
			for (ConnectionPtr connection : g->connections) {
				forceSim->addSpringItem(connection->getSpringItem());
			}
			for (RelationshipPtr relationship : g->relationships) {
				forceSim->addSpringItem(relationship->getSpringItem());
			}
			graphBounds = box2px(pixel2(0.0f), pixel2(0.0f));
			AlloyApplicationContext()->requestPack();
		}
		void DataFlow::add(const std::shared_ptr<Source>& node) {
			addNode(node);
		}
		void DataFlow::add(const std::shared_ptr<Destination>& node) {
			addNode(node);
		}
		void DataFlow::add(const std::shared_ptr<Data>& node) {
			addNode(node);
		}
		void DataFlow::add(const std::shared_ptr<Group>& node) {
			addNode(node);
		}
		void DataFlow::add(const std::shared_ptr<View>& node) {
			addNode(node);
		}
		void DataFlow::add(const std::shared_ptr<Compute>& node) {
			addNode(node);
		}
		void DataFlow::add(const std::shared_ptr<Connection>& connection) {
			addConnectionInternal(connection);
			if (onConnectionAction) {
				onConnectionAction(connection.get(), ConnectionAction::Add);
			}
		}
		void DataFlow::addConnectionInternal(const std::shared_ptr<Connection>& connection) {
			SpringItem* spring = new SpringItem(connection->source->getNode()->getForceItem(), connection->destination->getNode()->getForceItem(), -1.0f,
					2 * ForceSimulator::RADIUS);
			spring->gamma = 0.1f;
			spring->visible = false;
			spring->length = distance(spring->item1->location, spring->item2->location);
			connection->getSpringItem().reset(spring);
			routingLock.lock();
			forceSim->addSpringItem(connection->getSpringItem());
			data->connections.push_back(connection);
			routingLock.unlock();

		}
		Connection* DataFlow::closestConnection(const float2& pt, float tolernace) {
			for (int i = (int) data->connections.size() - 1; i >= 0; i--) {
				Connection* c = data->connections[i].get();
				float d = c->distance(pt);

				if (d < tolernace) {
					return c;
				}
			}
			return nullptr;
		}
		void DataFlow::setup() {
			data = MakeGroupNode(this->name);
			mouseDragNode = nullptr;
			mouseSelectedNode = nullptr;
			mouseOverNode = nullptr;
			dragBox = box2px(float2(0, 0), float2(0, 0));
			graphBounds = box2px(pixel2(0.0f), pixel2(0.0f));
			setRoundCorners(true);
			backgroundColor = MakeColor(AlloyApplicationContext()->theme.DARK);
			forceSim = ForceSimulatorPtr(new ForceSimulator("Force Simulator", CoordPX(0.0f, 0.0f), CoordPercent(1.0f, 1.0f)));
			DrawPtr pathsRegion = DrawPtr(new Draw("Paths", CoordPX(0.0f, 0.0f), CoordPercent(1.0f, 1.0f), [this](AlloyContext* context, const box2px& bounds) {
				for (RelationshipPtr& relationship : data->relationships) {
					relationship->draw(context,scale);
				}
				routingLock.lock();
				for (ConnectionPtr& connection : data->connections) {
					connection->draw(context, this);
				}
				routingLock.unlock();
				for (RelationshipPtr& relationship : data->relationships) {
					relationship->drawText(context,scale);
				}
			}));

			Composite::add(pathsRegion);
			Composite::add(forceSim);
			Application::addListener(this);
			forceSim->addForce(SpringForcePtr(new SpringForce()));
			forceSim->addForce(NBodyForcePtr(new NBodyForce()));
			forceSim->addForce(MakeShared<BuoyancyForce>());
			forceSim->addForce(boxForce = BoxForcePtr(new BoxForce(box2px(pixel2(0.0f, 0.0f), pixel2(1920, 1080)))));
			boxForce->setVisible(false);
			forceSim->addForce(DragForcePtr(new DragForce(0.001f)));
			forceSim->setZoom(1.0f);
			forceSim->setOffset(pixel2(0.0f, 0.0f));
			forceSim->onStep = [this](float stepSize) {
				AlloyContext* context = AlloyDefaultContext().get();
				if (context) {
					context->requestPack();
				}
			};
			errorMessage = MessageDialogPtr(
					new MessageDialog("Could not complete action because node(s) have external ports.", true, MessageOption::Okay, MessageType::Warning));
			errorMessage->onSelect = [this](MessageDialog* dialog) {
				errorMessage->setVisible(false);
				AlloyApplicationContext()->getGlassPane()->setVisible(false);
			};
			AlloyApplicationContext()->getGlassPane()->add(errorMessage);
		}
		void DataFlow::start() {
			if (enabled) {
				forceSim->start();
			}
		}
		void DataFlow::setSimulatorEnabled(bool b) {
			enabled = b;
			if (!enabled) {
				if (forceSim->isRunning()) {
					AlloyApplicationContext()->addDeferredTask([this]() {
						forceSim->stop();
					});
				}
			} else {
				forceSim->stop();
				if (isVisible()) {
					AlloyApplicationContext()->addDeferredTask([this]() {
						forceSim->start();
					});
				}
			}
		}
		void DataFlow::stop() {
			forceSim->stop();
		}
		void OutputMultiPort::insertValue(const std::shared_ptr<Packet>& packet, int index) {
			if (index == FrontIndex) {
				value.insert(value.begin(), packet);
			} else if (index == BackIndex) {
				value.push_back(packet);
			} else {
				if (index >= (int) value.size()) {
					value.push_back(packet);
				} else {
					value.insert(value.begin() + (size_t) index, packet);
				}
			}
		}
		void OutputMultiPort::setValue(const std::shared_ptr<Packet>& packet, int index) {
			if (index < 0) {
				throw std::runtime_error("Cannot set port value. Index out of range.");
			} else {
				if (index >= (int) value.size()) {
					value.resize(index + 1);
				}
				value[index] = packet;
			}
		}
		void InputMultiPort::insertValue(const std::shared_ptr<Packet>& packet, int index) {
			if (index == FrontIndex) {
				value.insert(value.begin(), packet);
			} else if (index == BackIndex) {
				value.push_back(packet);
			} else {
				if (index >= (int) value.size()) {
					value.push_back(packet);
				} else {
					value.insert(value.begin() + (size_t) index, packet);
				}
			}
		}
		void InputMultiPort::setValue(const std::shared_ptr<Packet>& packet, int index) {
			if (index < 0) {
				throw std::runtime_error("Cannot set port value. Index out of range.");
			} else {
				if (index >= (int) value.size()) {
					value.resize(index + 1);
				}
				value[index] = packet;
			}
		}
		void InputMultiPort::setValue(const std::shared_ptr<Packet>& packet) {
			insertAtBack(packet);
		}
		void OutputMultiPort::setValue(const std::shared_ptr<Packet>& packet) {
			insertAtBack(packet);
		}
		std::shared_ptr<Port> Port::getReference() {
			if (parent->inputPort.get() == this) {
				return parent->inputPort;
			}
			if (parent->outputPort.get() == this) {
				return parent->outputPort;
			}
			if (parent->childPort.get() == this) {
				return parent->childPort;
			}
			if (parent->parentPort.get() == this) {
				return parent->parentPort;
			}
			for (std::shared_ptr<Port> port : parent->inputPorts) {
				if (port.get() == this)
					return port;
			}
			for (std::shared_ptr<Port> port : parent->outputPorts) {
				if (port.get() == this)
					return port;
			}
			return std::shared_ptr<Port>();

		}
		void Port::setup() {
			position = CoordPX(0.0f, 0.0f);
			borderWidth = UnitPX(1.0f);
			id="p"+Node::MakeID();
			onMouseDown = [this](AlloyContext* context, const InputEvent& e) {
				if (e.button == GLFW_MOUSE_BUTTON_LEFT&&e.isDown()) {
					getGraph()->startConnection(this);
				}
				return false;
			};
		}
		DataFlow* Port::getGraph() const {
			return (parent != nullptr) ? parent->parentFlow : nullptr;
		}
		void InputPort::setup() {
			position = CoordPX(0.0f, 0.0f);
			dimensions = CoordPX(InputPort::DIMENSIONS);
		}
		void OutputPort::setup() {
			position = CoordPX(0.0f, 0.0f);
			dimensions = CoordPX(OutputPort::DIMENSIONS);
		}
		void ParentPort::setup() {
			position = CoordPX(0.0f, 0.0f);
			dimensions = CoordPX(OutputPort::DIMENSIONS);
		}
		void ChildPort::setup() {
			position = CoordPX(0.0f, 0.0f);
			dimensions = CoordPX(OutputPort::DIMENSIONS);
		}

		void Port::draw(AlloyContext* context) {
			NVGcontext* nvg = context->nvgContext;
			box2px bounds = getBounds();
			nvgFillColor(nvg, context->theme.LIGHT);
			nvgBeginPath(nvg);
			nvgEllipse(nvg, bounds.position.x + bounds.dimensions.x * 0.5f, bounds.position.y + bounds.dimensions.y * 0.5f, bounds.dimensions.x * 0.5f,
					bounds.dimensions.y * 0.5f);
			nvgFill(nvg);

		}
		void InputPort::pack(const pixel2& pos, const pixel2& dims, const double2& dpmm, double pixelRatio, bool clamp) {
			float scale = getNode()->getParentFlow()->getScale();
			this->dimensions = CoordPX(scale * InputPort::DIMENSIONS.x, scale * InputPort::DIMENSIONS.y);
			Region::pack(pos, dims, dpmm, pixelRatio, clamp);
		}
		void OutputPort::pack(const pixel2& pos, const pixel2& dims, const double2& dpmm, double pixelRatio, bool clamp) {
			float scale = getNode()->getParentFlow()->getScale();
			this->dimensions = CoordPX(scale * OutputPort::DIMENSIONS.x, scale * OutputPort::DIMENSIONS.y);
			Region::pack(pos, dims, dpmm, pixelRatio, clamp);
		}
		void ChildPort::pack(const pixel2& pos, const pixel2& dims, const double2& dpmm, double pixelRatio, bool clamp) {
			float scale = getNode()->getParentFlow()->getScale();
			this->dimensions = CoordPX(scale * InputPort::DIMENSIONS.x, scale * InputPort::DIMENSIONS.y);
			Region::pack(pos, dims, dpmm, pixelRatio, clamp);
		}
		void ParentPort::pack(const pixel2& pos, const pixel2& dims, const double2& dpmm, double pixelRatio, bool clamp) {
			float scale = getNode()->getParentFlow()->getScale();
			this->dimensions = CoordPX(scale * OutputPort::DIMENSIONS.x, scale * OutputPort::DIMENSIONS.y);
			Region::pack(pos, dims, dpmm, pixelRatio, clamp);
		}
		void InputPort::draw(AlloyContext* context) {
			if (!isVisible())
				return;

			NVGcontext* nvg = context->nvgContext;
			box2px bounds = getBounds();
			pixel lineWidth = borderWidth.toPixels(bounds.dimensions.y, context->dpmm.y, context->pixelRatio);
			bool over = false;
			if (isExternalized()) {
				if (context->isMouseOver(this)) {
					getGraph()->setCurrentPort(this);
					over = true;
				}
				nvgFillColor(nvg, context->theme.DARK);
				nvgStrokeColor(nvg, context->theme.LIGHT);
			} else {
				if (context->isMouseOver(this)) {
					nvgFillColor(nvg, context->theme.LIGHTEST);
					nvgStrokeColor(nvg, context->theme.LIGHTEST);
					context->setCursor(&Cursor::CrossHairs);
					getGraph()->setCurrentPort(this);
					over = true;
				} else {
					if (isConnected()) {
						nvgFillColor(nvg, context->theme.LIGHTEST);
						nvgStrokeColor(nvg, context->theme.LIGHTEST);
					} else {
						nvgFillColor(nvg, context->theme.LIGHT);
						nvgStrokeColor(nvg, context->theme.LIGHT);
					}
				}
			}
			nvgStrokeWidth(nvg, lineWidth);
			nvgBeginPath(nvg);
			nvgEllipse(nvg, bounds.position.x + bounds.dimensions.x * 0.5f, bounds.position.y + bounds.dimensions.y * 0.5f,
					bounds.dimensions.x * 0.5f - 0.5f * lineWidth, bounds.dimensions.y * 0.5f - 0.5f * lineWidth);
			nvgFill(nvg);
			nvgStroke(nvg);
			if (over) {
				nvgFontFaceId(nvg, context->getFontHandle(FontType::Normal));
				nvgFontSize(nvg, 18.0f);
				nvgTextAlign(nvg, NVG_ALIGN_BOTTOM | NVG_ALIGN_LEFT);
				nvgSave(nvg);
				nvgTranslate(nvg, bounds.position.x + bounds.dimensions.x, bounds.position.y);
				nvgRotate(nvg, -ALY_PI * 0.25f);
				aly::drawText(nvg, 0.0f, 0.0f, name.c_str(), FontStyle::Outline, context->theme.LIGHTEST, context->theme.DARK, nullptr);
				nvgRestore(nvg);
			}
		}
		void ParentPort::draw(AlloyContext* context) {
			NVGcontext* nvg = context->nvgContext;
			box2px bounds = getBounds();
			pixel lineWidth = borderWidth.toPixels(bounds.dimensions.y, context->dpmm.y, context->pixelRatio);
			bool over = false;
			if (context->isMouseOver(this)) {
				nvgFillColor(nvg, context->theme.LIGHTEST);
				nvgStrokeColor(nvg, context->theme.LIGHTEST);
				context->setCursor(&Cursor::CrossHairs);
				getGraph()->setCurrentPort(this);
				over = true;
			} else {
				if (isConnected()) {
					nvgFillColor(nvg, context->theme.LIGHTEST);
					nvgStrokeColor(nvg, context->theme.LIGHTEST);
				} else {
					nvgFillColor(nvg, context->theme.LIGHT);
					nvgStrokeColor(nvg, context->theme.LIGHT);
				}
			}
			nvgStrokeWidth(nvg, lineWidth);
			nvgBeginPath(nvg);
			nvgRect(nvg, bounds.position.x + 0.5f * lineWidth, bounds.position.y + 0.5f * lineWidth, bounds.dimensions.x - lineWidth,
					bounds.dimensions.y - lineWidth);
			nvgFill(nvg);
			nvgStroke(nvg);
			if (over) {
				nvgFontFaceId(nvg, context->getFontHandle(FontType::Normal));
				nvgFontSize(nvg, 18.0f);
				nvgTextAlign(nvg, NVG_ALIGN_BOTTOM | NVG_ALIGN_RIGHT);
				nvgSave(nvg);
				nvgTranslate(nvg, bounds.position.x + bounds.dimensions.x, bounds.position.y);
				aly::drawText(nvg, 0.0f, 0.0f, name.c_str(), FontStyle::Outline, context->theme.LIGHTEST, context->theme.DARK, nullptr);
				nvgRestore(nvg);
			}
		}
		void OutputPort::draw(AlloyContext* context) {
			if (!isVisible())
				return;
			NVGcontext* nvg = context->nvgContext;
			box2px bounds = getBounds();
			pixel lineWidth = borderWidth.toPixels(bounds.dimensions.y, context->dpmm.y, context->pixelRatio);
			bool over = false;
			if (isExternalized()) {
				if (context->isMouseOver(this)) {
					getGraph()->setCurrentPort(this);
					over = true;
				}
				nvgFillColor(nvg, context->theme.DARK);
				nvgStrokeColor(nvg, context->theme.LIGHT);
			} else {
				if (context->isMouseOver(this)) {
					nvgFillColor(nvg, context->theme.LIGHTEST);
					nvgStrokeColor(nvg, context->theme.LIGHTEST);
					context->setCursor(&Cursor::CrossHairs);
					getGraph()->setCurrentPort(this);
					over = true;
				} else {
					if (isConnected()) {
						nvgFillColor(nvg, context->theme.LIGHTEST);
						nvgStrokeColor(nvg, context->theme.LIGHTEST);
					} else {
						nvgFillColor(nvg, context->theme.LIGHT);
						nvgStrokeColor(nvg, context->theme.LIGHT);
					}
				}
			}
			nvgLineCap(nvg, NVG_ROUND);
			nvgLineJoin(nvg, NVG_BEVEL);
			nvgStrokeWidth(nvg, lineWidth);
			nvgBeginPath(nvg);

			nvgMoveTo(nvg, bounds.position.x + bounds.dimensions.x * 0.5f, bounds.position.y + bounds.dimensions.y - lineWidth);
			nvgLineTo(nvg, bounds.position.x + 0.5f, bounds.position.y + lineWidth * 0.5f);
			nvgLineTo(nvg, bounds.position.x + bounds.dimensions.x - 1.0f, bounds.position.y + lineWidth * 0.5f);
			nvgClosePath(nvg);
			nvgFill(nvg);
			nvgStroke(nvg);
			if (over) {
				nvgFontFaceId(nvg, context->getFontHandle(FontType::Normal));
				nvgFontSize(nvg, 18.0f);
				nvgTextAlign(nvg, NVG_ALIGN_TOP | NVG_ALIGN_LEFT);
				nvgSave(nvg);
				nvgTranslate(nvg, bounds.position.x + bounds.dimensions.x, bounds.position.y + bounds.dimensions.y);
				nvgRotate(nvg, ALY_PI * 0.25f);
				aly::drawText(nvg, 0.0f, 0.0f, name.c_str(), FontStyle::Outline, context->theme.LIGHTEST, context->theme.DARK, nullptr);
				nvgRestore(nvg);
			}
		}

		void ChildPort::draw(AlloyContext* context) {
			NVGcontext* nvg = context->nvgContext;
			box2px bounds = getBounds();
			pixel lineWidth = borderWidth.toPixels(bounds.dimensions.y, context->dpmm.y, context->pixelRatio);
			bool over = false;
			if (context->isMouseOver(this)) {
				nvgFillColor(nvg, context->theme.LIGHTEST);
				nvgStrokeColor(nvg, context->theme.LIGHTEST);
				context->setCursor(&Cursor::CrossHairs);
				getGraph()->setCurrentPort(this);
				over = true;
			} else {
				if (isConnected()) {
					nvgFillColor(nvg, context->theme.LIGHTEST);
					nvgStrokeColor(nvg, context->theme.LIGHTEST);
				} else {
					nvgFillColor(nvg, context->theme.LIGHT);
					nvgStrokeColor(nvg, context->theme.LIGHT);
				}
			}
			nvgLineCap(nvg, NVG_ROUND);
			nvgLineJoin(nvg, NVG_BEVEL);
			nvgStrokeWidth(nvg, lineWidth);
			nvgBeginPath(nvg);

			nvgMoveTo(nvg, bounds.position.x + bounds.dimensions.x - lineWidth, bounds.position.y + bounds.dimensions.y * 0.5f);
			nvgLineTo(nvg, bounds.position.x + lineWidth * 0.5f, bounds.position.y + 0.5f);
			nvgLineTo(nvg, bounds.position.x + lineWidth * 0.5f, bounds.position.y + bounds.dimensions.y - 1.0f);
			nvgClosePath(nvg);
			nvgFill(nvg);
			nvgStroke(nvg);
			if (over) {
				nvgFontFaceId(nvg, context->getFontHandle(FontType::Normal));
				nvgFontSize(nvg, 18.0f);
				nvgTextAlign(nvg, NVG_ALIGN_BOTTOM | NVG_ALIGN_LEFT);
				nvgSave(nvg);
				nvgTranslate(nvg, bounds.position.x, bounds.position.y);
				aly::drawText(nvg, 0.0f, 0.0f, name.c_str(), FontStyle::Outline, context->theme.LIGHTEST, context->theme.DARK, nullptr);
				nvgRestore(nvg);
			}
		}
		float2 Node::getLocation() const {
			return forceItem->location;
		}

		void Node::setLocation(const float2& pt) {
			forceItem->location = pt;
			forceItem->plocation = pt;
			forceItem->velocity = float2(0.0f);
		}
		bool Node::hasExternalInput() const {
			for (std::shared_ptr<InputPort> port : inputPorts) {
				if (port->isExternalized())
					return true;
			}
			return false;
		}
		bool Node::hasExternalOutput() const {
			for (std::shared_ptr<OutputPort> port : outputPorts) {
				if (port->isExternalized())
					return true;
			}
			return false;
		}
		std::shared_ptr<Port> InputPort::clone() const {
			std::shared_ptr<InputPort> port = std::shared_ptr<InputPort>(new InputPort(getName(), getLabel()));
			port->copyContentsFrom(this);
			return port;
		}
		std::shared_ptr<Port> OutputPort::clone() const {
			std::shared_ptr<OutputPort> port = std::shared_ptr<OutputPort>(new OutputPort(getName(), getLabel()));
			port->copyContentsFrom(this);
			return port;
		}
		std::shared_ptr<Port> ParentPort::clone() const {
			std::shared_ptr<ParentPort> port = std::shared_ptr<ParentPort>(new ParentPort(getName(), getLabel()));
			port->copyContentsFrom(this);
			return port;
		}
		std::shared_ptr<Port> ChildPort::clone() const {
			std::shared_ptr<ChildPort> port = std::shared_ptr<ChildPort>(new ChildPort(getName(), getLabel()));
			port->copyContentsFrom(this);
			return port;
		}

		std::shared_ptr<Node> Compute::clone() const {
			std::shared_ptr<Compute> node = std::shared_ptr<Compute>(new Compute(getName(), getLabel(), getLocation()));
			node->copyContentsFrom(this);
			node->parentFlow = parentFlow;
			node->fontSize = fontSize;
			node->textWidth = textWidth;
			node->centerOffset = centerOffset;
			node->setSelected(isSelected());
			for (InputPortPtr port : inputPorts) {
				node->add(std::dynamic_pointer_cast<InputPort>(port->clone()));
			}
			for (OutputPortPtr port : outputPorts) {
				node->add(std::dynamic_pointer_cast<OutputPort>(port->clone()));
			}
			return node;
		}
		std::shared_ptr<Node> Data::clone() const {
			std::shared_ptr<Data> node = std::shared_ptr<Data>(new Data(getName(), getLabel(), getLocation()));
			node->copyContentsFrom(this);
			node->parentFlow = parentFlow;
			node->fontSize = fontSize;
			node->textWidth = textWidth;
			node->centerOffset = centerOffset;
			node->setSelected(isSelected());
			for (InputPortPtr port : inputPorts) {
				node->add(std::dynamic_pointer_cast<InputPort>(port->clone()));
			}
			for (OutputPortPtr port : outputPorts) {
				node->add(std::dynamic_pointer_cast<OutputPort>(port->clone()));
			}
			return node;
		}
		std::shared_ptr<Node> View::clone() const {
			std::shared_ptr<View> node = std::shared_ptr<View>(new View(getName(), getLabel(), getLocation()));
			node->copyContentsFrom(this);
			node->parentFlow = parentFlow;
			node->fontSize = fontSize;
			node->textWidth = textWidth;
			node->centerOffset = centerOffset;
			node->setSelected(isSelected());
			for (InputPortPtr port : inputPorts) {
				node->add(std::dynamic_pointer_cast<InputPort>(port->clone()));
			}
			for (OutputPortPtr port : outputPorts) {
				node->add(std::dynamic_pointer_cast<OutputPort>(port->clone()));
			}
			return node;
		}
		std::shared_ptr<Node> Source::clone() const {
			std::shared_ptr<Source> node = std::shared_ptr<Source>(new Source(getName(), getLabel(), getLocation()));
			node->copyContentsFrom(this);
			node->parentFlow = parentFlow;
			node->fontSize = fontSize;
			node->textWidth = textWidth;
			node->centerOffset = centerOffset;
			node->setSelected(isSelected());
			return node;
		}
		std::shared_ptr<Node> Destination::clone() const {
			std::shared_ptr<Destination> node = std::shared_ptr<Destination>(new Destination(getName(), getLabel(), getLocation()));
			node->copyContentsFrom(this);
			node->parentFlow = parentFlow;
			node->fontSize = fontSize;
			node->textWidth = textWidth;
			node->centerOffset = centerOffset;
			node->setSelected(isSelected());
			for (InputPortPtr port : inputPorts) {
				node->add(std::dynamic_pointer_cast<InputPort>(port->clone()));
			}
			for (OutputPortPtr port : outputPorts) {
				node->add(std::dynamic_pointer_cast<OutputPort>(port->clone()));
			}
			return node;
		}
		std::shared_ptr<Node> Group::clone(bool selectedOnly) const {
			std::shared_ptr<Group> group = std::shared_ptr<Group>(new Group(getName(), getLabel(), getLocation()));
			group->copyContentsFrom(this);
			group->parentFlow = parentFlow;
			group->fontSize = fontSize;
			group->textWidth = textWidth;
			group->centerOffset = centerOffset;
			group->setSelected(isSelected());
			std::map<const Node*, std::shared_ptr<Node>> nodeMap;
			std::map<const Port*, std::shared_ptr<Port>> portMap;
			pixel2 center(0.0f);
			nodeMap[this]=group;
			for (NodePtr child : nodes) {
				if((selectedOnly&&child->isSelected())||!selectedOnly){
					NodePtr copy = child->clone();
					nodeMap[child.get()] = copy;
					for (int i = 0; i < (int) copy->getInputSize(); i++) {
						portMap[child->getInputPort(i).get()] = copy->getInputPort(i);
					}
					for (int i = 0; i < (int) copy->getOutputSize(); i++) {
						portMap[child->getOutputPort(i).get()] = copy->getOutputPort(i);
					}
					if (child->getParentPort().get() != nullptr) {
						portMap[child->getParentPort().get()] = copy->getParentPort();
					}
					if (child->getChildPort().get() != nullptr) {
						portMap[child->getChildPort().get()] = copy->getChildPort();
					}
					if (child->getOutputPort().get() != nullptr) {
						portMap[child->getOutputPort().get()] = copy->getOutputPort();
					}
					if (child->getInputPort().get() != nullptr) {
						portMap[child->getInputPort().get()] = copy->getInputPort();
					}
					center+=copy->getLocation();
					group->add(copy);
				}
			}
			if(group->nodes.size()>0){
				group->setLocation(center/(float)group->nodes.size());
			}
			if (group->parentPort.get() != nullptr) {
				portMap[parentPort.get()] = group->parentPort;
			}
			if (group->childPort.get() != nullptr) {
				portMap[childPort.get()] = group->childPort;
			}
			if (group->outputPort.get() != nullptr) {
				portMap[outputPort.get()] = group->outputPort;
			}
			if (group->inputPort.get() != nullptr) {
				portMap[inputPort.get()] = group->inputPort;
			}
			for (InputPortPtr port : inputPorts) {
				PortPtr copy = port->clone();
				group->add(std::dynamic_pointer_cast<InputPort>(copy));
				if (port->hasProxyOut()) {
					PortPtr proxy = portMap[port->getProxyOut().get()];
					copy->setProxyOut(proxy);
					proxy->setProxyIn(copy);
				}
				portMap[port.get()] = copy;
			}
			for (OutputPortPtr port : outputPorts) {
				PortPtr copy = port->clone();
				group->add(std::dynamic_pointer_cast<OutputPort>(copy));
				if (port->hasProxyOut()) {
					PortPtr proxy = portMap[port->getProxyOut().get()];
					copy->setProxyOut(proxy);
					proxy->setProxyIn(copy);
				}
				portMap[port.get()] = copy;
			}
			for (ConnectionPtr connection : connections) {
				if(selectedOnly){
					if(connection->source->getNode()->isSelected()&&connection->destination->getNode()->isSelected()){
						ConnectionPtr copy=MakeConnection(portMap[connection->source.get()], portMap[connection->destination.get()]);
						copy->copyContentsFrom(connection);
						group->add(copy);
					}
				} else {
					ConnectionPtr copy=MakeConnection(portMap[connection->source.get()], portMap[connection->destination.get()]);
					copy->copyContentsFrom(connection);
					group->add(copy);

				}
			}
			for (RelationshipPtr relation : relationships) {
				if(selectedOnly){
					if(relation->subject->isSelected()&&relation->object->isSelected()){
						group->add(MakeRelationship(nodeMap[relation->subject.get()], relation->predicate->clone(), nodeMap[relation->object.get()]));
					}
				} else {
					group->add(MakeRelationship(nodeMap[relation->subject.get()], relation->predicate->clone(), nodeMap[relation->object.get()]));
				}
			}
			return group;
		}

		void Node::pack(const pixel2& pos, const pixel2& dims, const double2& dpmm, double pixelRatio, bool clamp) {
			pixel2 dragOffset = getDragOffset();
			if (lengthL1(dragOffset) > 0 && parentFlow != nullptr) {
				std::lock_guard<std::mutex> lockMe(parentFlow->getForceSimulator()->getLock());
				forceItem->location += dragOffset / parentFlow->getScale();
				setDragOffset(pixel2(0.0f));
				pixel2 minPt = parentFlow->graphBounds.min();
				pixel2 maxPt = parentFlow->graphBounds.max();
				float minX = 1E30f;
				float maxX = -1E30f;
				float minY = 1E30f;
				float maxY = -1E30f;
				Node* minXnode = nullptr;
				Node* maxXnode = nullptr;
				Node* minYnode = nullptr;
				Node* maxYnode = nullptr;
				for (RegionPtr region : parentFlow->getChildren()) {
					Node* node = dynamic_cast<Node*>(region.get());
					if (node) {
						box2px box = box2px(node->getForceItem()->location - Node::DIMENSIONS * 0.5f, Node::DIMENSIONS);
						if (box.position.x < minX) {
							minX = box.position.x;
							minXnode = node;
						}
						if (box.position.x + box.dimensions.x > maxX) {
							maxX = box.position.x + box.dimensions.x;
							maxXnode = node;
						}
						if (box.position.y < minY) {
							minY = box.position.y;
							minYnode = node;
						}
						if (box.position.y + box.dimensions.y > maxY) {
							maxY = box.position.y + box.dimensions.y;
							maxYnode = node;
						}
					}
				}
				if (maxXnode == this) {
					maxPt.x = maxX;
				}
				if (minXnode == this) {
					minPt.x = minX;
				}
				if (maxYnode == this) {
					maxPt.y = maxY;
				}
				if (minYnode == this) {
					minPt.y = minY;
				}
				parentFlow->graphBounds = box2px(minPt, maxPt - minPt);
				parentFlow->forceSim->setBounds(CoordPX(parentFlow->graphBounds.position), CoordPX(parentFlow->graphBounds.dimensions));
				parentFlow->boxForce->setBounds(parentFlow->graphBounds);
				if (!AlloyApplicationContext()->isControlDown()) {
					for (ConnectionPtr connector : parentFlow->getConnections()) {
						connector->getSpringItem()->update();
					}
					for (RelationshipPtr relationship : parentFlow->getRelationships()) {
						relationship->getSpringItem()->update();
					}
				}
			}
			if (resizeFunc && parentFlow) {
				resizeFunc(parentFlow->getScale());
			}
			Composite::pack(pos, dims, dpmm, pixelRatio, clamp);
			centerOffset = nodeIcon->getBounds(false).center() - getBounds(false).position;
		}

		Node::Node(const std::string& name, const pixel2& pt) :
				Composite(name, CoordPX(0.0f, 0.0f), CoordPX(Node::DIMENSIONS)), label(name), parentFlow(nullptr) {
			forceItem = ForceItemPtr(new ForceItem(pt + Node::DIMENSIONS * 0.5f));
			setup();
		}
		Node::Node(const std::string& name, const std::string& label, const pixel2& pt) :
				Composite(name, CoordPX(0.0f, 0.0f), CoordPX(Node::DIMENSIONS)), label(label), parentFlow(nullptr) {
			forceItem = ForceItemPtr(new ForceItem(pt + Node::DIMENSIONS * 0.5f));
			setup();
		}
		box2px Node::getBounds(bool includeBounds) const {
			box2px box = Composite::getBounds(includeBounds);
			if (includeBounds) {
				box.position += parentFlow->getScale() * forceItem->location - centerOffset;
			}
			return box;
		}
		box2px Node::getCursorBounds(bool includeOffset) const {
			box2px box = (
					isDetached() ?
							getBounds(includeOffset) : box2px(bounds.position + parentFlow->getScale() * forceItem->location - centerOffset, bounds.dimensions));
			box.position += getDragOffset();
			if (parent != nullptr && (!isDetached() && includeOffset)) {
				box.position += parent->getDrawOffset();
				if (AlloyApplicationContext()->getOnTopRegion() != this) {
					box.intersect(parent->getCursorBounds());
				}
			}

			return box;

		}
		pixel2 Node::getDrawOffset() const {
			return Composite::getDrawOffset() + parentFlow->getScale() * forceItem->location - centerOffset;
		}
		void Node::draw(AlloyContext* context) {
			NVGcontext* nvg = context->nvgContext;
			box2px bounds = getBounds();
			const float scale = getParentFlow()->getScale();
			pixel lineWidth = borderWidth.toPixels(bounds.dimensions.y, context->dpmm.y, context->pixelRatio);
			nvgStrokeColor(nvg, context->theme.LIGHTER);
			nvgStrokeWidth(nvg, lineWidth);
			nvgLineCap(nvg, NVG_ROUND);
			box2px lbounds = labelRegion->getBounds();
			box2px cbounds = nodeIcon->getBounds();
			pixel2 labelStart = pixel2(cbounds.position.x + cbounds.dimensions.x * 0.5f, lbounds.position.y);
			pixel2 labelEnd = pixel2(bounds.position.x + bounds.dimensions.x, lbounds.position.y + lbounds.dimensions.y);
			nvgStrokeWidth(nvg, 1.0f);
			if (inputPorts.size() > 0 || outputPorts.size() > 0) {
				nvgStrokeColor(nvg, Color(context->theme.DARK.toLighter(0.25f)));
				nvgFillColor(nvg, context->theme.DARK.toLighter(0.25f).toSemiTransparent(NODE_ALPHA));
			} else {
				nvgStrokeColor(nvg, Color(COLOR_NONE));
				nvgFillColor(nvg, Color(context->theme.DARK.toSemiTransparent(NODE_ALPHA)));
			}
			nvgBeginPath(nvg);
			nvgRoundedRect(nvg, labelStart.x, labelStart.y, labelEnd.x - labelStart.x, labelEnd.y - labelStart.y, scale * context->theme.CORNER_RADIUS);
			nvgFill(nvg);
			nvgBeginPath(nvg);
			nvgRoundedRect(nvg, labelStart.x + 1.0f, labelStart.y + 1.0f, labelEnd.x - labelStart.x - 2.0f, labelEnd.y - labelStart.y - 2.0f,
					scale * context->theme.CORNER_RADIUS);
			nvgStroke(nvg);

			Composite::draw(context);
		}
		void View::draw(AlloyContext* context) {
			Node::draw(context);
			Port* p = getGraph()->getConnectingPort();
			if (p == inputPort.get()) {
				if (!inputPort->isVisible()) {
					context->requestPack();
				}
				inputPort->setVisible(true);
			} else {
				if (isMouseOver()) {
					if (!inputPort->isVisible()) {
						inputPort->setVisible(true);
						context->requestPack();
					}
				} else {
					if (!inputPort->isConnected())
						inputPort->setVisible(false);
				}
			}
		}
		void Compute::draw(AlloyContext* context) {
			Node::draw(context);
			Port* p = getGraph()->getConnectingPort();
			if (p == inputPort.get()) {
				if (!inputPort->isVisible()) {
					context->requestPack();
				}
				inputPort->setVisible(true);
			} else {
				if (isMouseOver()) {
					if (!inputPort->isVisible()) {
						inputPort->setVisible(true);
						context->requestPack();
					}
				} else {
					if (!inputPort->isConnected())
						inputPort->setVisible(false);
				}
			}
		}

		void Data::draw(AlloyContext* context) {
			Node::draw(context);
			Port* p = getGraph()->getConnectingPort();
			if (p == inputPort.get()) {
				if (!inputPort->isVisible()) {
					context->requestPack();
				}
				inputPort->setVisible(true);
				if (!outputPort->isConnected())
					outputPort->setVisible(false);
			} else if (p == outputPort.get()) {
				if (!outputPort->isVisible()) {
					context->requestPack();
				}
				outputPort->setVisible(true);
				if (!inputPort->isConnected())
					inputPort->setVisible(false);
			} else {
				if (isMouseOver()) {
					if (!inputPort->isVisible()) {
						inputPort->setVisible(true);
						context->requestPack();
					}
					if (!outputPort->isVisible()) {
						outputPort->setVisible(true);
					}
				} else {
					if (!inputPort->isConnected())
						inputPort->setVisible(false);

					if (!outputPort->isConnected())
						outputPort->setVisible(false);
				}
			}
		}
		std::shared_ptr<Connection> Group::getConnection(const std::string& srcId,const std::string& destId) const{
			for(ConnectionPtr con:connections){
				if(con->source->getReferenceId()==srcId&&con->destination->getReferenceId()==destId){
					return con;
				}
			}
			for(NodePtr child:nodes){
				GroupPtr g=std::dynamic_pointer_cast<Group>(child);
				if(g.get()!=nullptr){
					ConnectionPtr con=g->getConnection(srcId,destId);
					if(con.get()!=nullptr){
						return con;
					}
				}
			}
			return ConnectionPtr();
		}
		std::vector<std::shared_ptr<Node>> Group::getAllChildrenNodes() const {
			std::vector<std::shared_ptr<Node>> list=nodes;
			for(NodePtr child:nodes){
				if(child->getType()==NodeType::Group){
					GroupPtr g=std::dynamic_pointer_cast<Group>(child);
					std::vector<std::shared_ptr<Node>> l=g->getAllChildrenNodes();
					if(l.size()>0){
						list.insert(list.end(),l.begin(),l.end());
					}
				}
			}
			return list;
		}
		void Group::draw(AlloyContext* context) {
			Node::draw(context);
		}
		std::shared_ptr<Connection> Group::getConnection(Connection* con) {
			for (std::shared_ptr<Connection> connect : connections) {
				if (con == connect.get()) {
					return connect;
				}
			}
			for (std::shared_ptr<Node> node : nodes) {
				std::shared_ptr<Group> group = std::dynamic_pointer_cast<Group>(node);
				if (group.get() != nullptr) {
					std::shared_ptr<Connection> connect = group->getConnection(con);
					if (connect.get() != nullptr) {
						return connect;
					}
				}
			}
			return std::shared_ptr<Connection>();
		}
		Group* Group::getGroup(Connection* con) {
			for (std::shared_ptr<Connection> connect : connections) {
				if (con == connect.get()) {
					return this;
				}
			}
			for (std::shared_ptr<Node> node : nodes) {
				std::shared_ptr<Group> group = std::dynamic_pointer_cast<Group>(node);
				if (group.get() != nullptr) {
					Group* g = group->getGroup(con);
					if (g != nullptr) {
						return g;
					}
				}
			}
			return nullptr;
		}
		bool Group::remove(Connection* con) {
			for (auto iter = connections.begin(); iter != connections.end(); iter++) {
				std::shared_ptr<Connection> connect = *iter;
				if (connect.get() == con) {
					connections.erase(iter);
					return true;
				}
			}
			for (std::shared_ptr<Node> node : nodes) {
				std::shared_ptr<Group> group = std::dynamic_pointer_cast<Group>(node);
				if (group.get() != nullptr) {
					if (group->remove(con)) {
						return true;
					}
				}
			}
			return false;
		}
		bool Group::remove(Relationship* con) {
			for (auto iter =relationships.begin(); iter != relationships.end(); iter++) {
				std::shared_ptr<Relationship> connect = *iter;
				if (connect.get() == con) {
					relationships.erase(iter);
					return true;
				}
			}
			for (std::shared_ptr<Node> node : nodes) {
				std::shared_ptr<Group> group = std::dynamic_pointer_cast<Group>(node);
				if (group.get() != nullptr) {
					if (group->remove(con)) {
						return true;
					}
				}
			}
			return false;
		}

		void Source::draw(AlloyContext* context) {
			NVGcontext* nvg = context->nvgContext;
			box2px bounds = getBounds();
			const float scale = getParentFlow()->getScale();
			pixel lineWidth = borderWidth.toPixels(bounds.dimensions.y, context->dpmm.y, context->pixelRatio);
			nvgStrokeColor(nvg, context->theme.LIGHTER);
			nvgStrokeWidth(nvg, lineWidth);
			nvgLineCap(nvg, NVG_ROUND);
			box2px lbounds = labelRegion->getBounds();
			nvgStrokeWidth(nvg, 2.0f);

			nvgStrokeColor(nvg, Color(context->theme.DARK.toLighter(0.25f)));
			nvgFillColor(nvg, context->theme.DARK.toLighter(0.25f).toSemiTransparent(NODE_ALPHA));

			nvgBeginPath(nvg);
			nvgRoundedRect(nvg, lbounds.position.x, lbounds.position.y, lbounds.dimensions.x, lbounds.dimensions.y, scale * context->theme.CORNER_RADIUS);
			nvgFill(nvg);
			nvgBeginPath(nvg);
			nvgRoundedRect(nvg, lbounds.position.x + 1.0f, lbounds.position.y + 1.0f, lbounds.dimensions.x - 2.0f, lbounds.dimensions.y - 2.0f,
					scale * context->theme.CORNER_RADIUS);
			nvgStroke(nvg);

			Composite::draw(context);
		}
		void Destination::draw(AlloyContext* context) {
			NVGcontext* nvg = context->nvgContext;
			const float scale = getParentFlow()->getScale();
			box2px bounds = getBounds();
			pixel lineWidth = borderWidth.toPixels(bounds.dimensions.y, context->dpmm.y, context->pixelRatio);
			nvgStrokeColor(nvg, context->theme.LIGHTER);
			nvgStrokeWidth(nvg, lineWidth);
			nvgLineCap(nvg, NVG_ROUND);
			box2px lbounds = labelRegion->getBounds();
			nvgStrokeWidth(nvg, 2.0f);
			nvgStrokeColor(nvg, Color(context->theme.DARK.toLighter(0.25f)));
			nvgFillColor(nvg, context->theme.DARK.toLighter(0.25f).toSemiTransparent(NODE_ALPHA));

			nvgBeginPath(nvg);
			nvgRoundedRect(nvg, lbounds.position.x, lbounds.position.y, lbounds.dimensions.x, lbounds.dimensions.y, scale * context->theme.CORNER_RADIUS);
			nvgFill(nvg);
			nvgBeginPath(nvg);
			nvgRoundedRect(nvg, lbounds.position.x + 1.0f, lbounds.position.y + 1.0f, lbounds.dimensions.x - 2.0f, lbounds.dimensions.y - 2.0f,
					scale * context->theme.CORNER_RADIUS);
			nvgStroke(nvg);

			Composite::draw(context);
		}
		SpringItemPtr& Relationship::getSpringItem() {
			return springItem;
		}
		SpringItemPtr& Connection::getSpringItem() {
			return springItem;
		}
		bool Connection::remove() {
			return source->getGraph()->remove(this);
		}
		Direction Connection::getDirection() const {
			Direction direction;
			if (source->getType() == PortType::Parent) {
				direction = Direction::West;
			}
			else if (source->getType() == PortType::Child) {
				direction = Direction::East;
			}
			else if (source->getType() == PortType::Output) {
				direction = Direction::South;
			}
			else if (source->getType() == PortType::Input) {
				direction = Direction::North;
			}
			return direction;
		}
		void Connection::update() {
			if (springItem.get() == nullptr)
				springItem->update();
		}
		std::shared_ptr<Predicate> Predicate::clone() const {
			std::shared_ptr<Predicate> predicate = std::shared_ptr<Predicate>(new Predicate(name));
			return predicate;
		}
		std::string Predicate::getName() const {
			return name;
		}
		void Relationship::update() {
			if (springItem.get() == nullptr)
				springItem->update();
		}
		void Relationship::draw(AlloyContext* context, float scale) {
			pixel2 subjectPt = object->getCenter();
			pixel2 objectPt = subject->getCenter();
			NVGcontext* nvg = context->nvgContext;

			pixel2 vec = subjectPt - objectPt;
			float len = length(vec);
			const float arrowLength = 10 * scale;
			const float arrowWidth = 10 * scale;

			float r = subject->getRadius();
			if (len > 2 * r) {
				pixel2 mid = 0.5f * (objectPt + subjectPt);
				vec /= len;
				pixel2 ortho(-vec.y, vec.x);
				pixel2 pt1 = subjectPt - vec * r;

				nvgStrokeColor(nvg, context->theme.NEUTRAL);
				nvgStrokeWidth(nvg, std::max(scale * 4.0f, 1.0f));
				nvgLineCap(nvg, NVG_ROUND);
				nvgBeginPath(nvg);
				nvgMoveTo(nvg, objectPt.x, objectPt.y);
				subjectPt = pt1 - arrowLength * vec;
				nvgLineTo(nvg, subjectPt.x, subjectPt.y);
				nvgStroke(nvg);

				pixel2 pt2 = subjectPt + ortho * arrowWidth * 0.5f;
				pixel2 pt3 = subjectPt - ortho * arrowWidth * 0.5f;
				nvgFillColor(nvg, context->theme.NEUTRAL);
				nvgBeginPath(nvg);
				nvgMoveTo(nvg, pt1.x, pt1.y);
				nvgLineTo(nvg, pt2.x, pt2.y);
				nvgLineTo(nvg, pt3.x, pt3.y);
				nvgClosePath(nvg);
				nvgFill(nvg);
				if (object->isMouseOver() || subject->isMouseOver()) {
					nvgFontFaceId(nvg, context->getFont(FontType::Bold)->handle);
					const float th = 20 * scale;
					nvgFontSize(nvg, th - 2);
					nvgTextAlign(nvg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
					float tw = nvgTextBounds(nvg, 0, 0, predicate->getName().c_str(), nullptr, nullptr) + 2;
					nvgFillColor(nvg, context->theme.DARK.toSemiTransparent(NODE_ALPHA));
					nvgBeginPath(nvg);
					nvgRoundedRect(nvg, mid.x - tw * 0.5f, mid.y - th * 0.5f, tw, th, scale * context->theme.CORNER_RADIUS);
					nvgFill(nvg);
					aly::drawText(nvg, mid, predicate->getName(), FontStyle::Normal, context->theme.LIGHTEST, context->theme.DARK);
				}
			}
		}
		void Relationship::drawText(AlloyContext* context, float scale) {
			pixel2 scenter = object->getCenter();
			pixel2 ocenter = subject->getCenter();
			NVGcontext* nvg = context->nvgContext;
			float r = subject->getRadius();
			float len = distance(scenter, ocenter);
			if (len > 2 * r) {
				pixel2 mid = 0.5f * (scenter + ocenter);
				if (object->isMouseOver() || subject->isMouseOver()) {
					nvgFontFaceId(nvg, context->getFont(FontType::Bold)->handle);
					const float th = 20 * scale;
					nvgFontSize(nvg, th - 2);
					nvgTextAlign(nvg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
					float tw = nvgTextBounds(nvg, 0, 0, predicate->getName().c_str(), nullptr, nullptr) + 2;
					nvgFillColor(nvg, context->theme.DARK.toSemiTransparent(NODE_ALPHA));
					nvgBeginPath(nvg);
					nvgRoundedRect(nvg, mid.x - tw * 0.5f, mid.y - th * 0.5f, tw, th, scale * context->theme.CORNER_RADIUS);
					nvgFill(nvg);
					aly::drawText(nvg, mid, predicate->getName(), FontStyle::Normal, context->theme.LIGHTEST, context->theme.DARK);
				}
			}
		}
		void DataFlow::zoom(float f) {
			pixel2 pos = AlloyApplicationContext()->getCursorPosition();
			if (!bounds.contains(pos)) {
				pos = bounds.center();
			}
			setScale(scale, pos);
			AlloyApplicationContext()->requestPack();
		}
		void DataFlow::draw(AlloyContext* context) {
			mouseOverNode = nullptr;
			mouseSelectedNode = nullptr;
			const float nudge = context->theme.CORNER_RADIUS;
			if (selectedConnection != nullptr && context->getCursor() == nullptr && !context->getGlassPane()->isVisible()) {
				context->setCursor(&Cursor::CrossHairs);
			}
			for (std::shared_ptr<Region> child : children) {
				Node* node = dynamic_cast<Node*>(child.get());
				if (node) {
					if (context->isMouseOver(node, true)) {
						mouseOverNode = node;
						if (context->isMouseOver(node->nodeIcon.get())) {
							mouseSelectedNode = node;
						}
						break;
					}
				}
			}
			NVGcontext* nvg = context->nvgContext;

			box2px bounds = getBounds();
			float w = bounds.dimensions.x;
			float h = bounds.dimensions.y;
			pixel lineWidth = borderWidth.toPixels(bounds.dimensions.y, context->dpmm.y, context->pixelRatio);
			pushScissor(nvg, getCursorBounds());
			if (backgroundColor->a > 0) {
				nvgBeginPath(nvg);
				if (roundCorners) {
					nvgRoundedRect(nvg, bounds.position.x, bounds.position.y, bounds.dimensions.x, bounds.dimensions.y, context->theme.CORNER_RADIUS);
				} else {
					nvgRect(nvg, bounds.position.x, bounds.position.y, bounds.dimensions.x, bounds.dimensions.y);
				}
				nvgFillColor(nvg, *backgroundColor);
				nvgFill(nvg);
			}
			{
				nvgBeginPath(nvg);
				nvgFillColor(nvg, context->theme.DARKER);
				nvgStrokeWidth(nvg, 1.0f);
				float2 offset = getDrawOffset() + getBoundsPosition();
				nvgRect(nvg, scale * graphBounds.position.x + offset.x - scale * Node::DIMENSIONS.x * 0.5f,
						scale * graphBounds.position.y + offset.y - scale * Node::DIMENSIONS.y * 0.5f, scale * (graphBounds.dimensions.x + Node::DIMENSIONS.x),
						scale * (graphBounds.dimensions.y + Node::DIMENSIONS.y));
				nvgFill(nvg);
			}
			for (std::shared_ptr<Region>& region : children) {
				if (region->isVisible()) {
					region->draw(context);
				}
			}

			if (verticalScrollTrack.get() != nullptr) {
				if (isScrollEnabled()) {
					if (extents.dimensions.y > h) {
						verticalScrollTrack->draw(context);
						verticalScrollHandle->draw(context);
					} else {
						verticalScrollTrack->draw(context);
					}
					if (extents.dimensions.x > w) {
						horizontalScrollTrack->draw(context);
						horizontalScrollHandle->draw(context);
					} else {
						horizontalScrollTrack->draw(context);
					}
				}
			}
			popScissor(nvg);
			if (borderColor->a > 0) {

				nvgLineJoin(nvg, NVG_ROUND);
				nvgBeginPath(nvg);
				if (roundCorners) {
					nvgRoundedRect(nvg, bounds.position.x + lineWidth * 0.5f, bounds.position.y + lineWidth * 0.5f, bounds.dimensions.x - lineWidth,
							bounds.dimensions.y - lineWidth, context->theme.CORNER_RADIUS);
				} else {
					nvgRect(nvg, bounds.position.x + lineWidth * 0.5f, bounds.position.y + lineWidth * 0.5f, bounds.dimensions.x - lineWidth,
							bounds.dimensions.y - lineWidth);
				}
				nvgStrokeColor(nvg, *borderColor);
				nvgStrokeWidth(nvg, lineWidth);
				nvgStroke(nvg);
				nvgLineJoin(nvg, NVG_MITER);
			}

			if (connectingPort) {
				pixel2 cursor = context->cursorPosition;
				if (getBounds().contains(cursor)) {
					float2 offset = getDrawOffset();
					NVGcontext* nvg = context->nvgContext;
					nvgStrokeColor(nvg, context->theme.LIGHTEST);
					box2px bounds = connectingPort->getBounds();
					pixel2 start;
					Direction dir = Direction::Unkown;
					bool flipDir = false;
					switch (connectingPort->getType()) {
					case PortType::Input:
						start = pixel2(bounds.position.x + bounds.dimensions.x * 0.5f, bounds.position.y + nudge);
						dir = Direction::South;
						flipDir = true;
						break;
					case PortType::Output:
						start = pixel2(bounds.position.x + bounds.dimensions.x * 0.5f, bounds.position.y + bounds.dimensions.y - nudge);
						dir = Direction::South;
						break;
					case PortType::Child:
						start = pixel2(bounds.position.x + bounds.dimensions.x - nudge, bounds.position.y + bounds.dimensions.y * 0.5f);
						dir = Direction::East;
						break;
					case PortType::Parent:
						start = pixel2(bounds.position.x + nudge, bounds.position.y + bounds.dimensions.y * 0.5f);
						dir = Direction::East;
						flipDir = true;
						break;
					case PortType::Unknown:
						start = pixel2(bounds.position.x + bounds.dimensions.x * 0.5f, bounds.position.y + bounds.dimensions.y * 0.5f);
					}
					start -= offset;
					float2 end = cursor - offset;
					std::vector<float2> path;
					if (flipDir) {
						router.evaluate(path, end, start, dir);
					} else {
						router.evaluate(path, start, end, dir);
					}
					nvgLineCap(nvg, NVG_ROUND);
					nvgLineJoin(nvg, NVG_BEVEL);
					bool valid = true;
					if (currentPort != nullptr && context->isMouseOver(currentPort)) {
						nvgStrokeWidth(nvg, std::max(scale * 4.0f, 1.0f));
						if ((connectingPort->getType() == PortType::Input && currentPort->getType() == PortType::Output)
								|| (connectingPort->getType() == PortType::Output && currentPort->getType() == PortType::Input)) {
							if (onValidateConnection) {
								if (flipDir) {
									valid = onValidateConnection(currentPort, connectingPort);
								} else {
									valid = onValidateConnection(connectingPort, currentPort);
								}
							}
						} else if ((connectingPort->getType() == PortType::Parent && currentPort->getType() == PortType::Child)
								|| (connectingPort->getType() == PortType::Child && currentPort->getType() == PortType::Parent)) {
							if (onValidateConnection) {
								if (flipDir) {
									valid = onValidateConnection(currentPort, connectingPort);
								} else {
									valid = onValidateConnection(connectingPort, currentPort);
								}
							}
						} else {
							valid = false;
						}
					} else {
						nvgStrokeWidth(nvg, std::max(scale * 2.0f, 1.0f));
					}
					nvgStrokeColor(nvg, (valid) ? context->theme.LIGHTEST : Color(255, 128, 128));
					nvgBeginPath(nvg);
					float2 pt0 = offset + path.front();
					nvgMoveTo(nvg, pt0.x, pt0.y);
					for (int i = 1; i < (int) path.size() - 1; i++) {
						float2 pt1 = offset + path[i];
						float2 pt2 = offset + path[i + 1];
						float diff = 0.5f
								* std::min(std::max(std::abs(pt1.x - pt2.x), std::abs(pt1.y - pt2.y)),
										std::max(std::abs(pt1.x - pt0.x), std::abs(pt1.y - pt0.y)));
						if (diff < scale * context->theme.CORNER_RADIUS) {
							nvgLineTo(nvg, pt1.x, pt1.y);
						} else {
							nvgArcTo(nvg, pt1.x, pt1.y, pt2.x, pt2.y, scale * context->theme.CORNER_RADIUS);
						}
						pt0 = pt1;
					}
					pt0 = offset + path.back();
					nvgLineTo(nvg, pt0.x, pt0.y);
					nvgStroke(nvg);
				}

			}
			if (dragBox.dimensions.x > 0 && dragBox.dimensions.y > 0) {
				nvgBeginPath(nvg);
				nvgRect(nvg, dragBox.position.x, dragBox.position.y, dragBox.dimensions.x, dragBox.dimensions.y);
				nvgFillColor(nvg, Color(64, 64, 64, 128));
				nvgFill(nvg);

				nvgBeginPath(nvg);
				nvgRect(nvg, dragBox.position.x, dragBox.position.y, dragBox.dimensions.x, dragBox.dimensions.y);
				nvgStrokeWidth(nvg, 2.0f);
				nvgStrokeColor(nvg, Color(220, 220, 220));
				nvgStroke(nvg);
			}
		}
		void DataFlow::pack(const pixel2& pos, const pixel2& dims, const double2& dpmm, double pixelRatio, bool clamp) {
			currentDrawOffset = extents.position;
			Region::pack(pos, dims, dpmm, pixelRatio);
			box2px bounds = getBounds(false);
			pixel2 offset = cellPadding;
			pixel2 scrollExtent = pixel2(0.0f);
			for (std::shared_ptr<Region>& region : children) {
				if (!region->isVisible()) {
					continue;
				}
				if (orientation == Orientation::Vertical) {
					pixel2 pix = region->position.toPixels(bounds.dimensions, dpmm, pixelRatio);
					region->position = CoordPX(pix.x, offset.y);
				}
				if (orientation == Orientation::Horizontal) {
					pixel2 pix = region->position.toPixels(bounds.dimensions, dpmm, pixelRatio);
					region->position = CoordPX(offset.x, pix.y);
				}
				region->pack(bounds.position, bounds.dimensions, dpmm, pixelRatio);
				box2px cbounds = region->getBounds();
				if (orientation == Orientation::Horizontal) {
					offset.x += cellSpacing.x + cbounds.dimensions.x;

				}
				if (orientation == Orientation::Vertical) {
					offset.y += cellSpacing.y + cbounds.dimensions.y;
				}
				scrollExtent = aly::max(cbounds.dimensions + cbounds.position - this->bounds.position, scrollExtent);
			}
			extents.dimensions = scrollExtent;
			extents.position = currentDrawOffset;
			if (graphBounds.dimensions.x * graphBounds.dimensions.y == 0) {
				graphBounds = box2px(Node::DIMENSIONS * 0.5f, forceSim->getBoundsDimensions() - Node::DIMENSIONS);
				forceSim->setBounds(CoordPX(graphBounds.position), CoordPX(graphBounds.dimensions));
				boxForce->setBounds(graphBounds);
			}
			for (std::shared_ptr<Region>& region : children) {
				if (region->onPack)
					region->onPack();
			}
			routingLock.lock();
			router.update();
			for (ConnectionPtr& connect : data->connections) {
				router.evaluate(connect);
			}
			Connection* c = closestConnection((AlloyApplicationContext()->getCursorPosition() - getDrawOffset()), std::max(4.0f * scale, 1.0f));
			selectedConnection = c;
			routingLock.unlock();
		}
		void DataFlow::move(const std::shared_ptr<Node>& node, pixel2 position) {

		}

		void DataFlow::addNodes(const std::vector<std::shared_ptr<Node>>& nodes, const std::vector<std::shared_ptr<Connection>>& connections) {

		}
		void DataFlow::removeNodes(const std::vector<std::shared_ptr<Node>>& nodes, const std::vector<std::shared_ptr<Connection>>& connections) {

		}
		
		ActionGroupNodes::ActionGroupNodes(DataFlow* graph, const std::vector<std::shared_ptr<Node>>& nodes):ActionDataFlow(MakeString() << "Group " << nodes.size(), graph) {
			executeFunction = [this,nodes]() {
				this->group=this->graph->groupNodes(nodes);
				return true;
			};
			undoFunction = [this]() {
				this->graph->ungroupNodes(std::vector<NodePtr>{this->group});
				return true;
			};
		}
		ActionUnGroupNodes::ActionUnGroupNodes(DataFlow* graph, const std::vector<std::shared_ptr<Node>>& nodes):ActionDataFlow(MakeString() << "Ungroup " << nodes.size(), graph) {
			executeFunction = [this, nodes]() {
				this->ungrouped=this->graph->ungroupNodes(nodes);
				return true;
			};
			undoFunction = [this]() {
				this->graph->groupNodes(ungrouped);
				return true;
			};
		}

		ActionDataFlow::ActionDataFlow(const std::string& name, DataFlow* graph) :
				UndoableAction(name), graph(graph) {

		}
		ActionAddNode::ActionAddNode(DataFlow* graph, const std::shared_ptr<Node>& node) :
				ActionDataFlow(MakeString() << "Add " << node->getName(), graph) {
			executeFunction = [graph,node]() {
				graph->addNode(node);
				return (node->getParentFlow() == graph);
			};
			undoFunction = [graph, node]() {
				graph->removeNode(node);
				return (node->getParentFlow() == graph);
			};
		}
		ActionRemoveNodes::ActionRemoveNodes(DataFlow* graph, const std::vector<std::shared_ptr<Node>>& nodes,
				const std::vector<std::shared_ptr<Connection>>& connections) :
				ActionDataFlow(MakeString() << "Remove " << nodes.size() << "/" << connections.size(), graph) {
			executeFunction = [=]() {
				graph->removeNodes(nodes, connections);
				return true;
			};
			undoFunction = [=]() {
				graph->addNodes(nodes, connections);
				return true;
			};
		}
		ActionSelectNodes::ActionSelectNodes(DataFlow* graph, const std::vector<std::shared_ptr<Node>>& nodes):ActionDataFlow(MakeString() << "Select Nodes " << nodes.size(), graph) {
			executeFunction = [=]() {
				for (NodePtr n : nodes) {
					n->setSelected(true);
					if (this->graph->onNodeAction) {
						this->graph->onNodeAction(n.get(), NodeAction::Select);
					}
				}
				return true;
			};
			undoFunction = [=]() {
				for (NodePtr n : nodes) {
					n->setSelected(false);
					if (this->graph->onNodeAction) {
						this->graph->onNodeAction(n.get(), NodeAction::Select);
					}
				}
				return true;
			};
		}

		ActionSelectConnections::ActionSelectConnections(DataFlow* graph, const std::vector<std::shared_ptr<Connection>>& connections) : ActionDataFlow(MakeString() << "Select Connections " <<connections.size(), graph) {
			executeFunction = [=]() {
				for (ConnectionPtr n : connections) {
					n->setSelected(true);
					if (this->graph->onConnectionAction) {
						this->graph->onConnectionAction(n.get(), ConnectionAction::Select);
					}
				}
				return true;
			};
			undoFunction = [=]() {
				for (ConnectionPtr n : connections) {
					n->setSelected(false);
					if (this->graph->onConnectionAction) {
						this->graph->onConnectionAction(n.get(), ConnectionAction::Select);
					}
				}
				return true;
			};
		}
		ActionDeSelectConnections::ActionDeSelectConnections(DataFlow* graph, const std::vector<std::shared_ptr<Connection>>& connections) : ActionDataFlow(MakeString() << "De-Select Connections " << connections.size(), graph) {
			executeFunction = [=]() {
				for (ConnectionPtr n : connections) {
					n->setSelected(false);
					if (this->graph->onConnectionAction) {
						this->graph->onConnectionAction(n.get(), ConnectionAction::Select);
					}
				}
				return true;
			};
			undoFunction = [=]() {
				for (ConnectionPtr n : connections) {
					n->setSelected(true);
					if (this->graph->onConnectionAction) {
						this->graph->onConnectionAction(n.get(), ConnectionAction::Select);
					}
				}
				return true;
			};
		}
		ActionDeSelectNodes::ActionDeSelectNodes(DataFlow* graph, const std::vector<std::shared_ptr<Node>>& nodes) : ActionDataFlow(MakeString() << "Select Nodes " << nodes.size(), graph) {
			executeFunction = [=]() {
				for (NodePtr n : nodes) {
					n->setSelected(false);
					if (this->graph->onNodeAction) {
						this->graph->onNodeAction(n.get(), NodeAction::Select);
					}
				}
				return true;
			};
			undoFunction = [=]() {
				for (NodePtr n : nodes) {
					n->setSelected(true);
					if (this->graph->onNodeAction) {
						this->graph->onNodeAction(n.get(), NodeAction::Select);
					}
				}
				return true;
			};
		}
		ActionMoveNode::ActionMoveNode(DataFlow* graph, const std::shared_ptr<Node>& node, pixel2 position) :
				ActionDataFlow(MakeString() << "Move " << node->getName(), graph) {
			pixel2 oldPostion = node->getLocation();
			executeFunction = [=]() {
				node->setLocation(position);
				return (node->getParentFlow()==graph);
			};
			undoFunction = [=]() {
				node->setLocation(oldPostion);
				return (node->getParentFlow() == graph);
			};
		}
		ActionScaleGraph::ActionScaleGraph(DataFlow* graph, float scale, pixel2 position) :
				ActionDataFlow(MakeString() << "Scale " << graph->getName(), graph) {
			float oldScale = graph->getScale();
			executeFunction = [=]() {
				graph->setScale(scale,position);
				return true;
			};
			undoFunction = [=]() {
				graph->setScale(oldScale,position);
				return true;
			};
		}
		ActionAddConnection::ActionAddConnection(DataFlow* graph, const std::shared_ptr<Connection>& con):
			ActionDataFlow(MakeString() << "Add Connection " << graph->getName(), graph) {
			executeFunction = [=]() {
				graph->add(con);
				con->source->connect(con);
				con->destination->connect(con);
				return true;
			};
			undoFunction = [=]() {
				graph->remove(con.get());
				return true;
			};
		}
		ActionAddRelationship::ActionAddRelationship(DataFlow* graph, const std::shared_ptr<Relationship>& con):
			ActionDataFlow(MakeString() << "Add Relationship" << graph->getName(), graph) {
			executeFunction = [=]() {
				graph->add(con);
				return true;
			};
			undoFunction = [=]() {
				graph->remove(con.get());
				return true;
			};
		}
		ActionMoveGraph::ActionMoveGraph(DataFlow* graph, pixel2 position): ActionDataFlow(MakeString() << "Move Graph" << graph->getName(), graph) {
		}
		ActionModifyNode::ActionModifyNode(DataFlow* graph, const std::shared_ptr<Node>& node, const std::string& name) : ActionDataFlow(MakeString() << "Modify Name" << graph->getName(), graph) {
		}
	}
}