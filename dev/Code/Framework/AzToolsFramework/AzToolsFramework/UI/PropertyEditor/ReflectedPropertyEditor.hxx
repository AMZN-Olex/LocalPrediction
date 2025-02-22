/*
* All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
* its licensors.
*
* For complete copyright and license terms please see the LICENSE at the root of this
* distribution (the "License"). All use of this software is governed by the License,
* or, if provided, by the license below or the license accompanying this file. Do not
* remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*
*/

#ifndef REFLECTEDPROPERTYEDITOR_H
#define REFLECTEDPROPERTYEDITOR_H

#include <AzCore/base.h>
#include <AzCore/Memory/SystemAllocator.h>
#include "PropertyEditorAPI.h"
#include <QtWidgets/QWidget>
#include <QtWidgets/QFrame>

#pragma once

class QScrollArea;
class QLayout;
class QSpacerItem;
class QVBoxLayout;

namespace AZ
{
    struct ClassDataReflection;
    class SerializeContext;
}

namespace AzToolsFramework
{
    class ReflectedPropertyEditorState;
    class PropertyRowWidget;

    /**
     * the Reflected Property Editor is a Qt Control which you can place inside a GUI, which you then feed
     * a series of object(s) and instances.  Any object or instance with Editor reflection will then be editable
     * in the Reflected Property editor control, with the GUI arrangement specified in the edit reflection for
     * those objects.
     */
    class ReflectedPropertyEditor 
        : public QFrame
    {
        Q_OBJECT
    public:
        AZ_CLASS_ALLOCATOR(ReflectedPropertyEditor, AZ::SystemAllocator, 0);

        typedef AZStd::unordered_map<InstanceDataNode*, PropertyRowWidget*> WidgetList;

        ReflectedPropertyEditor(QWidget* pParent);
        virtual ~ReflectedPropertyEditor();

        void Setup(AZ::SerializeContext* context, IPropertyEditorNotify* ptrNotify, bool enableScrollbars, int propertyLabelWidth = 200);

        // allows disabling display of root container property widgets
        void SetHideRootProperties(bool hideRootProperties);

        bool AddInstance(void* instance, const AZ::Uuid& classId, void* aggregateInstance = nullptr, void* compareInstance = nullptr);
        void SetCompareInstance(void* instance, const AZ::Uuid& classId);
        void ClearInstances();
        template<class T>
        bool AddInstance(T* instance, void* aggregateInstance = nullptr, void* compareInstance = nullptr)
        {
            return AddInstance(instance, AZ::AzTypeInfo<T>().Uuid(), aggregateInstance, compareInstance);
        }

        void InvalidateAll(const char* filter = nullptr); // recreates the entire tree of properties.
        void InvalidateAttributesAndValues(); // re-reads all attributes, and all values.
        void InvalidateValues(); // just updates the values inside properties.

        void SetSavedStateKey(AZ::u32 key); // a settings key which is used to store and load the set of things that are expanded or not and other settings

        void QueueInvalidation(PropertyModificationRefreshLevel level);
        //will force any queued invalidations to happen immediately
        void ForceQueuedInvalidation();

        void CancelQueuedRefresh(); // Cancels any pending property refreshes

        void SetAutoResizeLabels(bool autoResizeLabels);

        InstanceDataNode* GetNodeFromWidget(QWidget* pTarget) const;
        PropertyRowWidget* GetWidgetFromNode(InstanceDataNode* node) const;

        void ExpandAll();
        void CollapseAll();

        const WidgetList& GetWidgets() const;

        int GetContentHeight() const;
        int GetMaxLabelWidth() const;

        void SetLabelAutoResizeMinimumWidth(int labelMinimumWidth);
        void SetLabelWidth(int labelWidth);

        void SetSelectionEnabled(bool selectionEnabled);
        void SelectInstance(InstanceDataNode* node);
        InstanceDataNode* GetSelectedInstance() const;

        QSize sizeHint() const override;

        using InstanceDataHierarchyCallBack = AZStd::function<void(AzToolsFramework::InstanceDataHierarchy& /*hierarchy*/)>;
        void EnumerateInstances(InstanceDataHierarchyCallBack enumerationCallback);

        void SetValueComparisonFunction(const InstanceDataHierarchy::ValueComparisonFunction& valueComparisonFunction);

        bool HasFilteredOutNodes() const;
        bool HasVisibleNodes() const;

        // if you want it to save its state, you need to give it a user settings label:
        //void SetSavedStateLabel(AZ::u32 label);
        //static void Reflect(const AZ::ClassDataReflection& reflection);

        void SetDynamicEditDataProvider(DynamicEditDataProvider provider);

        void SetSizeHintOffset(const QSize& offset);
        QSize GetSizeHintOffset() const;

        // Controls the indentation of the different levels in the tree. 
        // Note: do not call this method after adding instances, this indentation value is passed to PropertyRowWidgets 
        // during construction, therefore doesn't support updating dynamically after showing the widget.
        void SetTreeIndentation(int indentation);
        
        // Controls the indentation of the leafs in the tree. 
        // Note: do not call this method after adding instances, this indentation value is passed to PropertyRowWidgets 
        // during construction, therefore doesn't support updating dynamically after showing the widget.
        void SetLeafIndentation(int indentation);

    signals:
        void OnExpansionContractionDone();
    private:
        class Impl;
        std::unique_ptr<Impl> m_impl;

        virtual void paintEvent(QPaintEvent* event) override;
        int m_updateDepth = 0;

    private slots:
        void OnPropertyRowExpandedOrContracted(PropertyRowWidget* widget, InstanceDataNode* node, bool expanded, bool fromUserInteraction);
        void DoRefresh();
        void RecreateTabOrder();
        void OnPropertyRowRequestClear(PropertyRowWidget* widget, InstanceDataNode* node);
        void OnPropertyRowRequestContainerRemoveItem(PropertyRowWidget* widget, InstanceDataNode* node);
        void OnPropertyRowRequestContainerAddItem(PropertyRowWidget* widget, InstanceDataNode* node);
    };
}

#endif
