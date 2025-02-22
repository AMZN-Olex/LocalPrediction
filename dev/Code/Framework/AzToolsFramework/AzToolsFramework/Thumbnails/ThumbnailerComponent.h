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
#pragma once

#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/Component/Component.h>
#include <AzToolsFramework/Thumbnails/ThumbnailerBus.h>

namespace AzToolsFramework
{
    namespace Thumbnailer
    {
        class ThumbnailContext;

        class ThumbnailerComponent
            : public AZ::Component
            , public ThumbnailerRequestsBus::Handler
        {
        public:
            AZ_COMPONENT(ThumbnailerComponent, "{80090CA5-6A3A-4554-B5FE-A6D74ECB2D84}")

            ThumbnailerComponent();
            virtual ~ThumbnailerComponent();

            //////////////////////////////////////////////////////////////////////////
            // AZ::Component
            //////////////////////////////////////////////////////////////////////////
            void Activate() override;
            void Deactivate() override;
            static void Reflect(AZ::ReflectContext* context);

            //////////////////////////////////////////////////////////////////////////
            // ThumbnailerRequests
            //////////////////////////////////////////////////////////////////////////
            void RegisterContext(const char* contextName, int thumbnailSize) override;
            void RegisterThumbnailProvider(SharedThumbnailProvider provider, const char* contextName) override;
            SharedThumbnail GetThumbnail(SharedThumbnailKey thumbnailKey, const char* contextName) override;

            static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);

        private:
            AZStd::unordered_map<AZStd::string, AZStd::shared_ptr<ThumbnailContext>> m_thumbnails;
        };
    } // Thumbnailer
} // namespace AssetBrowser