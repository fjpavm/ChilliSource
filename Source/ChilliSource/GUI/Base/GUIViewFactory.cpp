//
//  GUIViewFactory.cpp
//  moFlo
//
//  Created by Scott Downie on 21/04/2011.
//  Copyright 2011 Tag Games. All rights reserved.
//

#include <ChilliSource/GUI/Base/GUIViewFactory.h>

#include <ChilliSource/Core/Base/Application.h>
#include <ChilliSource/Core/XML/rapidxml.hpp>

#include <ChilliSource/GUI/Button/HighlightButton.h>
#include <ChilliSource/GUI/Button/StretchableHighlightButton.h>
#include <ChilliSource/GUI/Button/ToggleButton.h>
#include <ChilliSource/GUI/Container/ScrollView.h>
#include <ChilliSource/GUI/Container/HorizontalList.h>
#include <ChilliSource/GUI/Container/GridView.h>
#include <ChilliSource/GUI/Container/VerticalList.h>
#include <ChilliSource/GUI/Image/ImageView.h>
#include <ChilliSource/GUI/Image/VerticalStretchableImage.h>
#include <ChilliSource/GUI/Image/StretchableImage.h>
#include <ChilliSource/GUI/Label/Label.h>
#include <ChilliSource/GUI/Label/EditableLabel.h>
#include <ChilliSource/GUI/Label/FormattedLabel.h>
#include <ChilliSource/GUI/ProgressBar/HorizontalProgressBar.h>
#include <ChilliSource/GUI/ProgressBar/HorizontalEndcappedProgressBar.h>
#include <ChilliSource/GUI/ProgressBar/VerticalEndcappedProgressBar.h>
#include <ChilliSource/GUI/ProgressBar/HorizontalClippingProgressBar.h>
#include <ChilliSource/GUI/SliderBar/HorizontalSliderBar.h>
#include <ChilliSource/GUI/SliderBar/VerticalSliderBar.h>

namespace ChilliSource
{
	namespace GUI
	{
		GUIViewFactory::MapDelegateToString GUIViewFactory::mmapDelegateToType;

		//--------------------------------------------------------
		/// Register Defaults
		///
		/// Register default views with the factory
		//--------------------------------------------------------
		void GUIViewFactory::RegisterDefaults()
		{
			Register<GUIView>("GUIView");
			Register<ScrollView, GUIView>("ScrollView");
			Register<ImageView, GUIView>("ImageView");
			Register<Label, GUIView>("Label");
			Register<StretchableImage, GUIView>("StretchableImage");
            Register<VerticalStretchableImage, GUIView>("VerticalStretchableImage");
			Register<HorizontalStretchableImage, GUIView>("HorizontalStretchableImage");
			Register<GridView, GUIView>("GridView");
			Register<EditableLabel, Label>("EditableLabel");
            Register<FormattedLabel, Label>("FormattedLabel");
			Register<SliderBar, GUIView>("SliderBar");
			Register<ProgressBar, GUIView>("ProgressBar");
			Register<Button, GUIView>("Button");
            Register<HorizontalList, GUIView>("HorizontalList");
            Register<VerticalList, GUIView>("VerticalList");
            
			//Defaults
			Register<HighlightButton, Button>("HighlightButton");
            Register<ToggleButton, Button>("ToggleButton");
			Register<HorizontalProgressBar, ProgressBar>("HorizontalProgressBar");
            Register<HorizontalSliderBar, SliderBar>("HorizontalSliderBar");
			Register<VerticalSliderBar, SliderBar>("VerticalSliderBar");
			Register<HorizontalEndcappedProgressBar, ProgressBar>("HorizontalEndcappedProgressBar");
            Register<VerticalEndcappedProgressBar, ProgressBar>("VerticalEndcappedProgressBar");
			Register<HorizontalClippingProgressBar, ProgressBar>("HorizontalClippingProgressBar");
            Register<StretchableHighlightButton, Button>("StretchableHighlightButton");
		}
		//--------------------------------------------------------
		/// Create GUI View 
		///
		/// Creates a widget view of the given type
		///
		/// @return GUI View
		//--------------------------------------------------------
		GUIViewSPtr GUIViewFactory::CreateGUIView(const std::string& instrTypeName, const Core::ParamDictionary& insParams)
		{
			//Create the UI of the given type
			MapDelegateToString::iterator it = mmapDelegateToType.find(instrTypeName);
			if(it != mmapDelegateToType.end())
			{
				return (it->second)(insParams);
			}

			return GUIViewSPtr();
		}
		//--------------------------------------------------------
		/// Create GUI View From Script
		///
		/// Creates a widget view from the given script file
		///
        /// @param The storage location to load from
        /// @param the file path
		/// @param Optional dynamic array to which views will be pushed
		/// @return GUI View
		//--------------------------------------------------------
		GUIViewSPtr GUIViewFactory::CreateGUIViewFromScript(Core::StorageLocation ineStorageLocation, const std::string& instrScriptFile, std::vector<GUIViewSPtr>* outpViews)
		{
			GUIViewSPtr pRootView;

            Core::FileStreamSPtr pFile = Core::Application::GetFileSystemPtr()->CreateFileStream(ineStorageLocation, instrScriptFile, Core::FileMode::k_read);
            assert(pFile);
            
            std::string strFile;
            pFile->GetAll(strFile);
			
            //Load the script
            rapidxml::xml_document<> xDoc;
            xDoc.parse<0>((char*)strFile.c_str());
            
			
            rapidxml::xml_node<> * pDocRoot = xDoc.first_node();

			if(pDocRoot && pDocRoot->isNamed("Layout"))
			{
				rapidxml::xml_node<> *  pViewElement = pDocRoot->first_node();
				if(pViewElement)
				{
					pRootView = CreateView(pViewElement, outpViews);
				}
			}

			return pRootView;
		}
		//--------------------------------------------------------
		/// Create View
		///
		/// Recursive function used to create subviews
		///
		/// @param View XML element
		/// @return Created view
		//--------------------------------------------------------
		GUIViewSPtr GUIViewFactory::CreateView(rapidxml::xml_node<>* inpViewElement, std::vector<GUIViewSPtr>* outpViews)
		{
			//Get the view type
			//Get the param dictionary config values
			GUIViewSPtr pView;
            std::string strType;
            std::string strSource;
            bool bExtern = false;
            Core::StorageLocation eStorageLoc;
            
            for(rapidxml::xml_attribute<> * pAttr = inpViewElement->first_attribute(); pAttr != nullptr; pAttr = pAttr->next_attribute())
            {
                if(pAttr->isNamed("Type"))
                {
                    strType = pAttr->value();
                }
                else if(pAttr->isNamed("Source"))
                {
                    bExtern = true;
                    strSource = pAttr->value();
                }
                else if(pAttr->isNamed("StorageLocation"))
                {
                    bExtern = true;
                    eStorageLoc = Core::GetStorageLocationFromString(pAttr->value());
                }
            }
            
			//Create the UI of the given type
			MapDelegateToString::iterator it = mmapDelegateToType.find(strType);
			if(it != mmapDelegateToType.end())
			{
				Core::ParamDictionary sParams;
				sParams.FromString(inpViewElement->value());

				//Lets load the given type!
				pView = (it->second)(sParams);
                
                if(bExtern)
                {
                    pView->AddSubview(CreateGUIViewFromScript(eStorageLoc, strSource));
                }

				if(outpViews)
				{
					outpViews->push_back(pView);
				}
                
				//Now we need to do some recursion and load any subviews
                for(rapidxml::xml_node<> * pNode = inpViewElement->first_node(); pNode != nullptr; pNode = pNode->next_sibling())
				{
                    if(pNode->type() == rapidxml::node_element)
                        pView->AddSubview(CreateView(pNode, outpViews));
				}

				return pView;
			}
			else
			{
				CS_ERROR_LOG("Cannot create GUI view of type: " + strType);
				return pView;
			}
		}
	}
}