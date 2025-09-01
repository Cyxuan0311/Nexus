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
    
    // 控制按钮布局
    controlsLayout_ = new QHBoxLayout();
    
    zoomInButton_ = new QPushButton("放大");
    zoomOutButton_ = new QPushButton("缩小");
    resetZoomButton_ = new QPushButton("重置视图");
    autoLayoutButton_ = new QPushButton("自动布局");
    
    layoutCombo_ = new QComboBox();
    layoutCombo_->addItem("层次布局");
    layoutCombo_->addItem("圆形布局");
    layoutCombo_->addItem("力导向布局");
    
    infoLabel_ = new QLabel("函数关系图");
    infoLabel_->setStyleSheet("color: #4EC9B0; font-weight: bold; font-size: 14px;");
    
    controlsLayout_->addWidget(infoLabel_);
    controlsLayout_->addStretch();
    controlsLayout_->addWidget(new QLabel("布局:"));
    controlsLayout_->addWidget(layoutCombo_);
    controlsLayout_->addWidget(autoLayoutButton_);
    controlsLayout_->addWidget(zoomInButton_);
    controlsLayout_->addWidget(zoomOutButton_);
    controlsLayout_->addWidget(resetZoomButton_);
    
    mainLayout_->addLayout(controlsLayout_);
    
    // 图形视图
    graphView_ = new QGraphicsView();
    graphView_->setMinimumHeight(400);
    mainLayout_->addWidget(graphView_);
    
    // 详细信息区域
    detailsArea_ = new QScrollArea();
    detailsArea_->setMaximumHeight(150);
    detailsLabel_ = new QLabel("点击函数节点查看详细信息");
    detailsLabel_->setWordWrap(true);
    detailsLabel_->setStyleSheet("color: #CCCCCC; padding: 10px; background-color: #2D2D30;");
    detailsArea_->setWidget(detailsLabel_);
    mainLayout_->addWidget(detailsArea_);
    
    // 连接信号
    connect(zoomInButton_, &QPushButton::clicked, this, &FunctionGraphView::zoomIn);
    connect(zoomOutButton_, &QPushButton::clicked, this, &FunctionGraphView::zoomOut);
    connect(resetZoomButton_, &QPushButton::clicked, this, &FunctionGraphView::resetZoom);
    connect(autoLayoutButton_, &QPushButton::clicked, this, &FunctionGraphView::autoLayout);
    connect(layoutCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &FunctionGraphView::onLayoutTypeChanged);
    
    // 设置样式
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
        infoLabel_->setText("没有找到函数定义");
        return;
    }
    
    createNodes();
    createEdges();
    layoutNodes();
    
    // 适配视图
    scene_->setSceneRect(scene_->itemsBoundingRect());
    graphView_->fitInView(scene_->sceneRect(), Qt::KeepAspectRatio);
    
    infoLabel_->setText(QString("已显示 %1 个函数，%2 个调用关系")
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
        
        // 连接节点点击事件
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
    
    // 更新连接线
    for (auto* edge : edges_) {
        edge->updateLine();
    }
}

void FunctionGraphView::layoutHierarchical() {
    if (nodes_.empty()) return;
    
    // 找到根节点（没有被其他函数调用的函数）
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
    
    // 如果没有根节点，随机选择一些
    if (rootFunctions.empty()) {
        for (int i = 0; i < std::min(3, (int)functions_.size()); ++i) {
            rootFunctions.push_back(functions_[i].name);
        }
    }
    
    // 层次布局
    std::map<std::string, int> levels;
    std::queue<std::string> queue;
    
    // 初始化根节点
    for (const std::string& root : rootFunctions) {
        levels[root] = 0;
        queue.push(root);
    }
    
    // BFS 分配层级
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
    
    // 为未分配层级的节点分配层级
    for (const auto& func : functions_) {
        if (levels.find(func.name) == levels.end()) {
            levels[func.name] = 0;
        }
    }
    
    // 按层级组织节点
    std::map<int, std::vector<std::string>> levelGroups;
    for (const auto& pair : levels) {
        levelGroups[pair.second].push_back(pair.first);
    }
    
    // 定位节点
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
    
    // 更新连接线
    for (auto* edge : edges_) {
        edge->updateLine();
    }
}

void FunctionGraphView::layoutForceDirected() {
    if (nodes_.empty()) return;
    
    // 简单的力导向布局
    const int iterations = 100;
    const qreal repulsion = 50000.0;
    const qreal attraction = 0.01;
    const qreal damping = 0.9;
    
    // 随机初始位置
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(-200, 200);
    
    for (auto& pair : nodes_) {
        pair.second->setPos(dis(gen), dis(gen));
    }
    
    for (int iter = 0; iter < iterations; ++iter) {
        std::map<FunctionNode*, QPointF> forces;
        
        // 初始化力
        for (auto& pair : nodes_) {
            forces[pair.second] = QPointF(0, 0);
        }
        
        // 计算排斥力
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
        
        // 计算吸引力（通过连接）
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
        
        // 应用力
        for (auto& pair : nodes_) {
            QPointF pos = pair.second->pos();
            QPointF force = forces[pair.second];
            pos += force * damping;
            pair.second->setPos(pos);
        }
    }
    
    // 更新连接线
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
    // 取消之前选中的节点
    if (selectedNode_) {
        selectedNode_->setSelected(false);
    }
    
    selectedNode_ = node;
    node->setSelected(true);
    
    // 显示函数详细信息
    const CppFunction& func = node->getFunction();
    QString details = QString(
        "<h3 style='color: #4EC9B0;'>%1</h3>"
        "<p><b>返回类型:</b> %2</p>"
        "<p><b>行号:</b> %3</p>"
        "<p><b>参数:</b></p>"
        "<ul>%4</ul>"
        "<p><b>调用的函数:</b> %5</p>"
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
    
    // 设置节点样式
    setBrush(QBrush(QColor("#3C3C3C")));
    setPen(QPen(QColor("#007ACC"), 2));
    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemIsSelectable);
    
    // 创建文本
    textItem_ = new QGraphicsTextItem(QString::fromStdString(function.name), this);
    textItem_->setDefaultTextColor(QColor("#CCCCCC"));
    textItem_->setFont(QFont("Arial", 10, QFont::Bold));
    
    // 居中文本
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
    setZValue(-1); // 在节点下方绘制
    
    updateLine();
    createArrowHead();
}

void FunctionEdge::updateLine() {
    if (!fromNode_ || !toNode_) return;
    
    QPointF startPos = fromNode_->pos();
    QPointF endPos = toNode_->pos();
    
    // 计算节点边缘的连接点
    QPointF direction = endPos - startPos;
    qreal length = sqrt(direction.x() * direction.x() + direction.y() * direction.y());
    
    if (length > 0) {
        direction /= length;
        QPointF startPoint = startPos + direction * 40; // 节点半径
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
 
#include "function_graph_view.moc" 