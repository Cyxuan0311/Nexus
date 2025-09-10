#include "function_graph_view.h"
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QtMath>
#include <QApplication>
#include <QMessageBox>
#include <QTextEdit>
#include <algorithm>
#include <random>
#include <queue>
#include <set>

FunctionGraphView::FunctionGraphView(QWidget* parent)
    : QWidget(parent), selectedNode_(nullptr), nodeRadius_(40), nodeSpacing_(150), levelSpacing_(120) {
    setupUI();
    
    scene_ = new QGraphicsScene(this);
    graphView_->setScene(scene_);
    graphView_->setRenderHint(QPainter::Antialiasing);
    graphView_->setDragMode(QGraphicsView::ScrollHandDrag);
}

FunctionGraphView::~FunctionGraphView() {
    clearGraph();
}

void FunctionGraphView::setupUI() {
    mainLayout_ = new QVBoxLayout(this);
    
    // Control button layout
    controlsLayout_ = new QHBoxLayout();
    
    zoomInButton_ = new QPushButton("Zoom In");
    zoomOutButton_ = new QPushButton("Zoom Out");
    resetZoomButton_ = new QPushButton("Reset View");
    autoLayoutButton_ = new QPushButton("Auto Layout");
    
    layoutCombo_ = new QComboBox();
    layoutCombo_->addItem("Hierarchical Layout");
    layoutCombo_->addItem("Circular Layout");
    layoutCombo_->addItem("Force-Directed Layout");
    
    infoLabel_ = new QLabel("Function Graph");
    infoLabel_->setStyleSheet("color: #4EC9B0; font-weight: bold; font-size: 14px;");
    
    controlsLayout_->addWidget(infoLabel_);
    controlsLayout_->addStretch();
    controlsLayout_->addWidget(new QLabel("Layout:"));
    controlsLayout_->addWidget(layoutCombo_);
    controlsLayout_->addWidget(autoLayoutButton_);
    controlsLayout_->addWidget(zoomInButton_);
    controlsLayout_->addWidget(zoomOutButton_);
    controlsLayout_->addWidget(resetZoomButton_);
    
    mainLayout_->addLayout(controlsLayout_);
    
    // Graphics view
    graphView_ = new QGraphicsView();
    graphView_->setMinimumHeight(400);
    mainLayout_->addWidget(graphView_);
    
    // Details area
    detailsArea_ = new QScrollArea();
    detailsArea_->setMaximumHeight(150);
    detailsLabel_ = new QLabel("Click function node to view details");
    detailsLabel_->setWordWrap(true);
    detailsLabel_->setStyleSheet("color: #CCCCCC; padding: 10px; background-color: #2D2D30;");
    detailsArea_->setWidget(detailsLabel_);
    mainLayout_->addWidget(detailsArea_);
    
    // Connect signals
    connect(zoomInButton_, &QPushButton::clicked, this, &FunctionGraphView::zoomIn);
    connect(zoomOutButton_, &QPushButton::clicked, this, &FunctionGraphView::zoomOut);
    connect(resetZoomButton_, &QPushButton::clicked, this, &FunctionGraphView::resetZoom);
    connect(autoLayoutButton_, &QPushButton::clicked, this, &FunctionGraphView::autoLayout);
    connect(layoutCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &FunctionGraphView::onLayoutTypeChanged);
    
    // Set styles
    setStyleSheet(R"(
        QPushButton {
            background-color: #0E639C;
            border: 1px solid #007ACC;
            color: white;
            padding: 6px 12px;
            font-weight: bold;
            border-radius: 3px;
        }
        QPushButton:hover {
            background-color: #1177BB;
        }
        QPushButton:pressed {
            background-color: #005A9E;
        }
        QComboBox {
            background-color: #3C3C3C;
            border: 1px solid #5A5A5A;
            color: #CCCCCC;
            padding: 4px 8px;
            border-radius: 3px;
        }
        QComboBox::drop-down {
            border: none;
        }
        QComboBox::down-arrow {
            color: #CCCCCC;
        }
        QGraphicsView {
            background-color: #1E1E1E;
            border: 1px solid #3E3E42;
        }
    )");
}

void FunctionGraphView::setParserData(const CppParser& parser) {
    functions_ = parser.getFunctions();
    classes_ = parser.getClasses();
    functionCalls_ = parser.getFunctionCalls();
}

void FunctionGraphView::generateGraph() {
    clearGraph();
    
    if (functions_.empty()) {
        infoLabel_->setText("No function definitions found");
        return;
    }
    
    createNodes();
    createEdges();
    layoutNodes();
    
    // Fit view
    scene_->setSceneRect(scene_->itemsBoundingRect());
    graphView_->fitInView(scene_->sceneRect(), Qt::KeepAspectRatio);
    
    infoLabel_->setText(QString("Displaying %1 functions, %2 call relationships")
                       .arg(functions_.size())
                       .arg(edges_.size()));
}

void FunctionGraphView::clearGraph() {
    scene_->clear();
    nodes_.clear();
    edges_.clear();
    selectedNode_ = nullptr;
}

void FunctionGraphView::createNodes() {
    for (const auto& func : functions_) {
        FunctionNode* node = new FunctionNode(func);
        nodes_[func.name] = node;
        scene_->addItem(node);
        
        // Connect node click events
        connect(node, &FunctionNode::nodeClicked, this, &FunctionGraphView::onNodeClicked);
    }
}

void FunctionGraphView::createEdges() {
    for (const auto& callPair : functionCalls_) {
        const std::string& callerName = callPair.first;
        const auto& calledFunctions = callPair.second;
        
        auto callerIt = nodes_.find(callerName);
        if (callerIt == nodes_.end()) continue;
        
        FunctionNode* callerNode = callerIt->second;
        
        for (const std::string& calledName : calledFunctions) {
            auto calledIt = nodes_.find(calledName);
            if (calledIt != nodes_.end()) {
                FunctionNode* calledNode = calledIt->second;
                FunctionEdge* edge = new FunctionEdge(callerNode, calledNode);
                edges_.push_back(edge);
                scene_->addItem(edge);
            }
        }
    }
}

void FunctionGraphView::layoutNodes() {
    switch (layoutCombo_->currentIndex()) {
        case 0: layoutHierarchical(); break;
        case 1: layoutCircular(); break;
        case 2: layoutForceDirected(); break;
        default: layoutHierarchical(); break;
    }
}

void FunctionGraphView::layoutCircular() {
    if (nodes_.empty()) return;
    
    const qreal centerX = 0;
    const qreal centerY = 0;
    const qreal radius = nodeSpacing_ * nodes_.size() / (2 * M_PI);
    
    int i = 0;
    for (auto& pair : nodes_) {
        qreal angle = 2 * M_PI * i / nodes_.size();
        qreal x = centerX + radius * cos(angle);
        qreal y = centerY + radius * sin(angle);
        
        pair.second->setPos(x, y);
        ++i;
    }
    
    // Update connection lines
    for (auto* edge : edges_) {
        edge->updateLine();
    }
}

void FunctionGraphView::layoutHierarchical() {
    if (nodes_.empty()) return;
    
    // Find root nodes (functions not called by other functions)
    std::set<std::string> calledFunctions;
    for (const auto& pair : functionCalls_) {
        for (const std::string& called : pair.second) {
            calledFunctions.insert(called);
        }
    }
    
    std::vector<std::string> rootFunctions;
    for (const auto& func : functions_) {
        if (calledFunctions.find(func.name) == calledFunctions.end()) {
            rootFunctions.push_back(func.name);
        }
    }
    
    // If no root nodes, randomly select some
    if (rootFunctions.empty()) {
        for (int i = 0; i < std::min(3, (int)functions_.size()); ++i) {
            rootFunctions.push_back(functions_[i].name);
        }
    }
    
    // Hierarchical layout
    std::map<std::string, int> levels;
    std::queue<std::string> queue;
    
    // Initialize root nodes
    for (const std::string& root : rootFunctions) {
        levels[root] = 0;
        queue.push(root);
    }
    
    // BFS assign levels
    while (!queue.empty()) {
        std::string current = queue.front();
        queue.pop();
        
        auto it = functionCalls_.find(current);
        if (it != functionCalls_.end()) {
            for (const std::string& called : it->second) {
                if (levels.find(called) == levels.end()) {
                    levels[called] = levels[current] + 1;
                    queue.push(called);
                }
            }
        }
    }
    
    // Assign levels to unassigned nodes
    for (const auto& func : functions_) {
        if (levels.find(func.name) == levels.end()) {
            levels[func.name] = 0;
        }
    }
    
    // Organize nodes by level
    std::map<int, std::vector<std::string>> levelGroups;
    for (const auto& pair : levels) {
        levelGroups[pair.second].push_back(pair.first);
    }
    
    // Position nodes
    for (const auto& levelPair : levelGroups) {
        int level = levelPair.first;
        const auto& functionNames = levelPair.second;
        
        qreal y = level * levelSpacing_;
        qreal totalWidth = (functionNames.size() - 1) * nodeSpacing_;
        qreal startX = -totalWidth / 2;
        
        for (size_t i = 0; i < functionNames.size(); ++i) {
            qreal x = startX + i * nodeSpacing_;
            auto nodeIt = nodes_.find(functionNames[i]);
            if (nodeIt != nodes_.end()) {
                nodeIt->second->setPos(x, y);
            }
        }
    }
    
    // Update connection lines
    for (auto* edge : edges_) {
        edge->updateLine();
    }
}

void FunctionGraphView::layoutForceDirected() {
    if (nodes_.empty()) return;
    
    // Simple force-directed layout
    const int iterations = 100;
    const qreal repulsion = 50000.0;
    const qreal attraction = 0.01;
    const qreal damping = 0.9;
    
    // Random initial positions
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(-200, 200);
    
    for (auto& pair : nodes_) {
        pair.second->setPos(dis(gen), dis(gen));
    }
    
    for (int iter = 0; iter < iterations; ++iter) {
        std::map<FunctionNode*, QPointF> forces;
        
        // Initialize forces
        for (auto& pair : nodes_) {
            forces[pair.second] = QPointF(0, 0);
        }
        
        // Calculate repulsion forces
        for (auto& pair1 : nodes_) {
            for (auto& pair2 : nodes_) {
                if (pair1.second == pair2.second) continue;
                
                QPointF pos1 = pair1.second->pos();
                QPointF pos2 = pair2.second->pos();
                QPointF delta = pos1 - pos2;
                qreal distance = sqrt(delta.x() * delta.x() + delta.y() * delta.y());
                
                if (distance > 0) {
                    QPointF force = delta / distance * repulsion / (distance * distance);
                    forces[pair1.second] += force;
                }
            }
        }
        
        // Calculate attraction forces (through connections)
        for (auto* edge : edges_) {
            QPointF pos1 = edge->fromNode_->pos();
            QPointF pos2 = edge->toNode_->pos();
            QPointF delta = pos2 - pos1;
            qreal distance = sqrt(delta.x() * delta.x() + delta.y() * delta.y());
            
            if (distance > 0) {
                QPointF force = delta * attraction * distance;
                forces[edge->fromNode_] += force;
                forces[edge->toNode_] -= force;
            }
        }
        
        // Apply forces
        for (auto& pair : nodes_) {
            QPointF pos = pair.second->pos();
            QPointF force = forces[pair.second];
            pos += force * damping;
            pair.second->setPos(pos);
        }
    }
    
    // Update connection lines
    for (auto* edge : edges_) {
        edge->updateLine();
    }
}

void FunctionGraphView::zoomIn() {
    graphView_->scale(1.2, 1.2);
}

void FunctionGraphView::zoomOut() {
    graphView_->scale(0.8, 0.8);
}

void FunctionGraphView::resetZoom() {
    graphView_->resetTransform();
    graphView_->fitInView(scene_->sceneRect(), Qt::KeepAspectRatio);
}

void FunctionGraphView::autoLayout() {
    layoutNodes();
}

void FunctionGraphView::onNodeClicked(FunctionNode* node) {
    // Deselect previously selected node
    if (selectedNode_) {
        selectedNode_->setSelected(false);
    }
    
    selectedNode_ = node;
    node->setSelected(true);
    
    // Show function details
    const CppFunction& func = node->getFunction();
    QString details = QString(
        "<h3 style='color: #4EC9B0;'>%1</h3>"
        "<p><b>Return Type:</b> %2</p>"
        "<p><b>Line Number:</b> %3</p>"
        "<p><b>Parameters:</b></p>"
        "<ul>%4</ul>"
        "<p><b>Called Functions:</b> %5</p>"
    ).arg(QString::fromStdString(func.name))
     .arg(QString::fromStdString(func.returnType))
     .arg(func.lineNumber);
    
    QString paramList;
    for (const auto& param : func.parameters) {
        paramList += QString("<li>%1 %2</li>")
                    .arg(QString::fromStdString(param.type))
                    .arg(QString::fromStdString(param.name));
    }
    
    QString calledFuncs;
    auto it = functionCalls_.find(func.name);
    if (it != functionCalls_.end()) {
        QStringList called;
        for (const std::string& name : it->second) {
            called << QString::fromStdString(name);
        }
        calledFuncs = called.join(", ");
    }
    
    details = details.arg(paramList).arg(calledFuncs);
    detailsLabel_->setText(details);
}

void FunctionGraphView::onLayoutTypeChanged(int index) {
    Q_UNUSED(index)
    layoutNodes();
}

void FunctionGraphView::showFunctionDetails(const QString& functionName) {
    auto it = nodes_.find(functionName.toStdString());
    if (it != nodes_.end()) {
        onNodeClicked(it->second);
        graphView_->centerOn(it->second);
    }
}

// FunctionNode 实现
FunctionNode::FunctionNode(const CppFunction& function, QGraphicsItem* parent)
    : QObject(), QGraphicsEllipseItem(-40, -40, 80, 80, parent), function_(function), 
      isSelected_(false), isHighlighted_(false) {
    
    // Set node style
    setBrush(QBrush(QColor("#3C3C3C")));
    setPen(QPen(QColor("#007ACC"), 2));
    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemIsSelectable);
    
    // Create text
    textItem_ = new QGraphicsTextItem(QString::fromStdString(function.name), this);
    textItem_->setDefaultTextColor(QColor("#CCCCCC"));
    textItem_->setFont(QFont("Arial", 10, QFont::Bold));
    
    // Center text
    QRectF textRect = textItem_->boundingRect();
    textItem_->setPos(-textRect.width() / 2, -textRect.height() / 2);
}

void FunctionNode::setSelected(bool selected) {
    isSelected_ = selected;
    update();
}

void FunctionNode::updatePosition(const QPointF& pos) {
    setPos(pos);
}

void FunctionNode::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    QGraphicsEllipseItem::mousePressEvent(event);
    emit nodeClicked(this);
}

void FunctionNode::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    QGraphicsEllipseItem::mouseReleaseEvent(event);
}

void FunctionNode::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option)
    Q_UNUSED(widget)
    
    QColor fillColor = QColor("#3C3C3C");
    QColor borderColor = QColor("#007ACC");
    
    if (isSelected_) {
        fillColor = QColor("#4EC9B0");
        borderColor = QColor("#6CDFE7");
    } else if (isHighlighted_) {
        fillColor = QColor("#5A5A5A");
        borderColor = QColor("#FFD700");
    }
    
    painter->setBrush(QBrush(fillColor));
    painter->setPen(QPen(borderColor, 2));
    painter->drawEllipse(rect());
}

// FunctionEdge 实现
FunctionEdge::FunctionEdge(FunctionNode* from, FunctionNode* to, QGraphicsItem* parent)
    : QGraphicsLineItem(parent), fromNode_(from), toNode_(to), isHighlighted_(false) {
    
    setPen(QPen(QColor("#6A6A6A"), 1));
    setZValue(-1); // Draw below nodes
    
    updateLine();
    createArrowHead();
}

void FunctionEdge::updateLine() {
    if (!fromNode_ || !toNode_) return;
    
    QPointF startPos = fromNode_->pos();
    QPointF endPos = toNode_->pos();
    
    // Calculate connection points at node edges
    QPointF direction = endPos - startPos;
    qreal length = sqrt(direction.x() * direction.x() + direction.y() * direction.y());
    
    if (length > 0) {
        direction /= length;
        QPointF startPoint = startPos + direction * 40; // Node radius
        QPointF endPoint = endPos - direction * 40;
        
        setLine(QLineF(startPoint, endPoint));
        
        if (arrowHead_) {
            arrowHead_->setPolygon(createArrowPolygon(endPoint, startPoint));
        }
    }
}

void FunctionEdge::setHighlighted(bool highlighted) {
    isHighlighted_ = highlighted;
    update();
}

void FunctionEdge::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option)
    Q_UNUSED(widget)
    
    QColor lineColor = isHighlighted_ ? QColor("#FFD700") : QColor("#6A6A6A");
    QPen pen(lineColor, isHighlighted_ ? 2 : 1);
    painter->setPen(pen);
    painter->drawLine(line());
}

void FunctionEdge::createArrowHead() {
    arrowHead_ = new QGraphicsPolygonItem(this);
    arrowHead_->setBrush(QBrush(QColor("#6A6A6A")));
    arrowHead_->setPen(QPen(QColor("#6A6A6A")));
}

QPolygonF FunctionEdge::createArrowPolygon(const QPointF& tip, const QPointF& tail) {
    QPointF direction = tip - tail;
    qreal length = sqrt(direction.x() * direction.x() + direction.y() * direction.y());
    
    if (length == 0) return QPolygonF();
    
    direction /= length;
    QPointF normal(-direction.y(), direction.x());
    
    const qreal arrowLength = 10;
    const qreal arrowWidth = 6;
    
    QPolygonF arrow;
    arrow << tip;
    arrow << tip - direction * arrowLength + normal * arrowWidth;
    arrow << tip - direction * arrowLength - normal * arrowWidth;
    
    return arrow;
} 