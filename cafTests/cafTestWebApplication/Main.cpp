#include "WPopupMenuWrapper.h"

#include "MainWindow.h"

#include <Wt/WApplication.h>
#include <Wt/WBootstrapTheme.h>
#include <Wt/WEnvironment.h>
#include <Wt/WFitLayout.h>

#include <memory>

class ThemedApp : public Wt::WApplication
{
public:
    // Constructor
    ThemedApp(const Wt::WEnvironment& env)
        : Wt::WApplication(env)
    {
        this->enableUpdates(true);
        auto theme = std::make_shared<Wt::WBootstrapTheme>();
        theme->setVersion(Wt::BootstrapVersion::v3);
        theme->setFormControlStyleEnabled(true);
        theme->setResponsive(true);
        setTheme(theme);
        std::unique_ptr<MainWindow> window = std::make_unique<MainWindow>();
        setTitle("Ceetron Application Framework Test Application");
        auto layout = root()->setLayout<Wt::WFitLayout>(std::make_unique<Wt::WFitLayout>());
        layout->addWidget<MainWindow>(std::move(window));
        this->triggerUpdate();
    }
};

std::unique_ptr<Wt::WApplication> createApplication(const Wt::WEnvironment& env)
{
    std::unique_ptr<ThemedApp> app = std::make_unique<ThemedApp>(env);
    return app;
}

int main(int argc, char* argv[])
{
    return WRun(argc, argv, &createApplication);
}
