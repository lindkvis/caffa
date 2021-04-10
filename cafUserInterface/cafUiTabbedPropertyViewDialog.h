#pragma once

#include <QDialog>

namespace caf
{
class Object;
class UiPropertyView;
} // namespace caf

class QDialogButtonBox;
class QWidget;
class QString;
class QStringList;

namespace caf
{
class UiTabbedPropertyViewDialog : public QDialog
{
public:
    UiTabbedPropertyViewDialog( caf::Object* object, const QStringList& tabLabels, const QString& windowTitle, QWidget* parent );
    ~UiTabbedPropertyViewDialog() override;

    QDialogButtonBox* dialogButtonBox();

protected:
    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

private:
    std::vector<UiPropertyView*> m_propertyViewTabs;
    QDialogButtonBox*            m_dialogButtonBox;
};

} // namespace caf
