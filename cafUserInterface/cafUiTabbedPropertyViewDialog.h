#pragma once

#include <QDialog>

namespace caffa
{
class Object;
class UiPropertyView;
} // namespace caffa

class QDialogButtonBox;
class QWidget;
class QString;
class QStringList;

namespace caffa
{
class UiTabbedPropertyViewDialog : public QDialog
{
public:
    UiTabbedPropertyViewDialog( caffa::Object* object, const QStringList& tabLabels, const QString& windowTitle, QWidget* parent );
    ~UiTabbedPropertyViewDialog() override;

    QDialogButtonBox* dialogButtonBox();

protected:
    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

private:
    std::vector<UiPropertyView*> m_propertyViewTabs;
    QDialogButtonBox*            m_dialogButtonBox;
};

} // namespace caffa
