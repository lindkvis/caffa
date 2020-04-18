//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2014 Ceetron Solutions AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################

#include "cafCmdFeature.h"

#include "cafAssert.h"
#include "cafCmdExecCommandManager.h"
#include "cafCmdFeatureManager.h"
#include "cafQActionWrapper.h"

#include "cafPdmUiModelChangeDetector.h"

#include <QApplication>

namespace caf
{

const ActionCreatorInterface* CmdFeature::s_actionCreator = QActionCreator::instance();

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
CmdFeature::CmdFeature()
    : m_triggerModelChange(true)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
CmdFeature::~CmdFeature()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::shared_ptr<ActionWrapper> CmdFeature::action()
{
    return this->actionWithCustomText(QString(""));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::shared_ptr<ActionWrapper> CmdFeature::actionWithCustomText(const QString& customText)
{
    return actionWithUserData(customText, QVariant());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::shared_ptr<ActionWrapper> CmdFeature::actionWithUserData(const QString& customText, const QVariant& userData)
{
    std::shared_ptr<ActionWrapper> action = nullptr;

    std::map<QString, std::shared_ptr<ActionWrapper>>::iterator it;
    it = m_customTextToActionMap.find(customText);

    if (it != m_customTextToActionMap.end() && it->second != NULL)
    {
        action = it->second;
    }
    else
    {
        action = s_actionCreator->createAction("", this);
        action->connect(std::bind(&CmdFeature::actionTriggered, this, std::placeholders::_1));
        m_customTextToActionMap[customText]= action;
    }

    if (!userData.isNull())
    {
        action->setData(userData);
    }

    this->setupActionLook(action.get());    
    if (!customText.isEmpty())
    {
        action->setText(customText);
    }

    return action;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void CmdFeature::refreshEnabledState()
{
    std::map<QString, std::shared_ptr<ActionWrapper>>::iterator it;
    bool isEnabled = this->isCommandEnabled();

    for (it = m_customTextToActionMap.begin(); it != m_customTextToActionMap.end(); ++it)
    {
        it->second->setEnabled(isEnabled);
        CAF_ASSERT(it->second->isEnabled() == isEnabled);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void CmdFeature::refreshCheckedState()
{
    std::map<QString, std::shared_ptr<ActionWrapper>>::iterator it;
    bool isChecked = this->isCommandChecked();

    for (it = m_customTextToActionMap.begin(); it != m_customTextToActionMap.end(); ++it)
    {
        std::shared_ptr<ActionWrapper> act = it->second;
        if (act->isCheckable())
        {
            it->second->setChecked(isChecked);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool CmdFeature::canFeatureBeExecuted()
{
    return this->isCommandEnabled();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void CmdFeature::applyShortcutWithHintToAction(ActionWrapper* action, const QKeySequence::StandardKey& keySequence)
{
    action->setShortcut(keySequence);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void CmdFeature::setActionCreator(const ActionCreatorInterface* actionCreator)
{
    s_actionCreator = actionCreator;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void CmdFeature::actionTriggered(bool isChecked)
{
    this->onActionTriggered(isChecked);

    if (m_triggerModelChange)
    {
        caf::PdmUiModelChangeDetector::instance()->setModelChanged();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool CmdFeature::isCommandChecked()
{
    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void CmdFeature::disableModelChangeContribution()
{
    m_triggerModelChange = false;
}

//--------------------------------------------------------------------------------------------------
/// Returns action user data.
/// May be called from onActionTriggered only
//--------------------------------------------------------------------------------------------------
const QVariant CmdFeature::userData() const
{
    ActionWrapper* action = qobject_cast<ActionWrapper*>(sender());
    CAF_ASSERT(action);

    return action->data();
}

} // end namespace caf
