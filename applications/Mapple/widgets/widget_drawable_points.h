#ifndef WIDGET_DRAWABLE_POINTS_H
#define WIDGET_DRAWABLE_POINTS_H

#include <unordered_map>
#include <QWidget>

#include "widgets/widget_drawable.h"

namespace Ui {
    class WidgetPointsDrawable;
}

namespace easy3d {
    class Model;
    class PointsDrawable;
}


class WidgetPointsDrawable : public WidgetDrawable {
Q_OBJECT

public:
    explicit WidgetPointsDrawable(QWidget *parent);

    ~WidgetPointsDrawable() override;

    // update the panel to be consistent with the drawable's rendering parameters
    void updatePanel() override;

    easy3d::PointsDrawable *drawable();

public slots:

    void setDrawableVisible(bool);
    void setActiveDrawable(const QString &);

    void setPointSize(double);
    void setImposterStyle(const QString &);

    void setLighting(const QString &);

    void setColorScheme(const QString &);
    void setDefaultColor();
    void setDistinctBackColor(bool);
    void setBackColor();

    void setHighlight(bool);
    void setHighlightMin(int);
    void setHighlightMax(int);

    void setScalarFieldStyle(int);
    void setScalarFieldClamp(bool);
    void setScalarFieldClampLower(int);
    void setScalarFieldClampUpper(int);

    void setVectorField(const QString &);
    void setVectorFieldScale(double);

private:
    void connectAll();
    void disconnectAll();

    void disableUnavailableOptions();

    void updateVectorFieldBuffer(easy3d::Model *model, const std::string &name);

    // model depended stuff
    std::vector<std::string> vectorFields(const easy3d::Model *model);

private:
    Ui::WidgetPointsDrawable *ui;

private:
    // the state of the rendering panel
    struct State {
        State() : initialized(false), coloring("uniform color"),
                  texture_file(""), scalar_style(0), clamp_value(true), clamp_value_lower(5),
                  clamp_value_upper(5), vector_field("disabled"), vector_field_scale(1.0) {
        }

        bool initialized;
        std::string coloring;
        std::string texture_file;
        int scalar_style;
        bool clamp_value;
        int clamp_value_lower;
        int clamp_value_upper;
        std::string vector_field;
        double vector_field_scale;
    };

    std::unordered_map<easy3d::PointsDrawable *, State> states_;
};

#endif // WIDGET_DRAWABLE_POINTS_H
