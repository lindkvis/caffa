#pragma once

#include <QDialog>

namespace caf
{
class Object;
class PdmUiPropertyView;
} // namespace caf

class QDialogButtonBox;
class QWidget;
class QString;
class QStringList;

namespace caf
{
class PdmUiTabbedPropertyViewDialog : public QDialog
{
public:
    PdmUiTabbedPropertyViewDialog( caf::Object*       object,
                                   const QStringList& tabLabels,
                                   const QString&     windowTitle,
                                   QWidget*           parent );
    ~PdmUiTabbedPropertyViewDialog() override;

    QDialogButtonBox* dialogButtonBox();

protected:
    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

private:
    std::vector<PdmUiPropertyView*> m_propertyViewTabs;
    QDialogButtonBox*               m_dialogButtonBox;
};

} // namespace caf
