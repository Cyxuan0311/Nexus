#ifndef FUNCTION_GRAPH_VIEW_H
#define FUNCTION_GRAPH_VIEW_H

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsTextItem>
#include <QGraphicsLineItem>
#include <QGraphicsPolygonItem>
#include <QPen>
#include <QBrush>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QScrollArea>
#include <QComboBox>
#include <map>
#include <vector>
#include "cpp_parser.h"

class FunctionNode;
class FunctionEdge;

class FunctionGraphView : public QWidget {
    Q_OBJECT

public:
    explicit FunctionGraphView(QWidget* parent = nullptr);
    ~FunctionGraphView();
    
    // 设置 C++ 解析数据
    void setParserData(const CppParser& parser);
    
    // 生成函数关系图
    void generateGraph();
    
    // 清空图形
    void clearGraph();

public slots:
    void zoomIn();
    void zoomOut();
    void resetZoom();
    void autoLayout();
    void showFunctionDetails(const QString& functionName);

private slots:
    void onNodeClicked(FunctionNode* node);
    void onLayoutTypeChanged(int index);

private:
    void setupUI();
    void createNodes();
    void createEdges();
    void layoutNodes();
    void layoutCircular();
    void layoutHierarchical();
    void layoutForceDirected();
    
    // UI 组件
    QVBoxLayout* mainLayout_;
    QHBoxLayout* controlsLayout_;
    QGraphicsView* graphView_;
    QGraphicsScene* scene_;
    QPushButton* zoomInButton_;
    QPushButton* zoomOutButton_;
    QPushButton* resetZoomButton_;
    QPushButton* autoLayoutButton_;
    QComboBox* layoutCombo_;
    QLabel* infoLabel_;
    QScrollArea* detailsArea_;
    QLabel* detailsLabel_;
    
    // 数据
    std::vector<CppFunction> functions_;
    std::vector<CppClass> classes_;
    std::map<std::string, std::vector<std::string>> functionCalls_;
    
    // 图形节点
    std::map<std::string, FunctionNode*> nodes_;
    std::vector<FunctionEdge*> edges_;
    
    // 当前选中的节点
    FunctionNode* selectedNode_;
    
    // 布局参数
    int nodeRadius_;
    int nodeSpacing_;
    int levelSpacing_;
};

// 函数节点类
class FunctionNode : public QObject, public QGraphicsEllipseItem {
    Q_OBJECT
    
public:
    FunctionNode(const CppFunction& function, QGraphicsItem* parent = nullptr);
    
    const CppFunction& getFunction() const { return function_; }
    void setSelected(bool selected);
    void updatePosition(const QPointF& pos);
    
    // 高亮相关函数
    void highlightCalledFunctions(bool highlight);
    void highlightCallingFunctions(bool highlight);
    
protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

signals:
    void nodeClicked(FunctionNode* node);

private:
    CppFunction function_;
    QGraphicsTextItem* textItem_;
    bool isSelected_;
    bool isHighlighted_;
};

// 函数调用边类
class FunctionEdge : public QGraphicsLineItem {
public:
    FunctionEdge(FunctionNode* from, FunctionNode* to, QGraphicsItem* parent = nullptr);
    
    void updateLine();
    void setHighlighted(bool highlighted);
    
    FunctionNode* fromNode_;
    FunctionNode* toNode_;
    
protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

private:
    bool isHighlighted_;
    QGraphicsPolygonItem* arrowHead_;
    
    void createArrowHead();
    QPolygonF createArrowPolygon(const QPointF& tip, const QPointF& tail);
};

#endif // FUNCTION_GRAPH_VIEW_H 