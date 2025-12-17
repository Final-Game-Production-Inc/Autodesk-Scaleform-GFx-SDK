/**************************************************************************

Filename    :   AS3_Global.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_AS3_Global_H
#define INC_AS3_Global_H

#include "Abc/AS3_Abc_Type.h"

#define SF_AS3_VERSION_MULTITOUCH
#ifdef GFX_ENABLE_XML
    #define SF_AS3_VERSION_XML
#endif
#define SF_AS3_VERSION_SOUND
#define SF_AS3_VERSION_NET
#define SF_AS3_VERSION_VIDEO
#define SF_AS3_VERSION_IME
#define SF_AS3_VERSION_REST
#define SF_AS3_VERSION_SHARED_OBJECT
#define SF_AS3_VERSION_AIR
#if defined(SF_ENABLE_ANE) && defined(SF_AS3_VERSION_AIR) 
	#define SF_AS3_VERSION_EXTENSION_CONTEXT
#endif

#ifdef GFX_AS3_SUPPORT

namespace Scaleform { namespace GFx { namespace AS3 
{

namespace fl
{
    extern const ClassInfo ObjectCI;
    extern const ClassInfo StringCI;
    extern const ClassInfo BooleanCI;
    extern const ClassInfo int_CI;
    extern const ClassInfo uintCI;
    extern const ClassInfo NumberCI;
    extern const ClassInfo NamespaceCI;
    extern const ClassInfo ArrayCI;
    extern const ClassInfo DomainCI;
}

namespace fl_vec
{
    extern const ClassInfo VectorCI;
    extern const ClassInfo Vector_objectCI;
    extern const ClassInfo Vector_intCI;
    extern const ClassInfo Vector_uintCI;
    extern const ClassInfo Vector_doubleCI;
    extern const ClassInfo Vector_StringCI;
}

namespace fl_media
{
    extern const ClassInfo VideoCI;
}

namespace fl_net
{
    extern const ClassInfo NetStreamCI;
}

namespace fl_accessibility
{
    extern const ClassInfo ISearchableTextCI;
    extern const ClassInfo ISimpleTextSelectionCI;
    extern const ClassInfo AccessibilityCI;
    extern const ClassInfo AccessibilityImplementationCI;
    extern const ClassInfo AccessibilityPropertiesCI;
}

namespace fl
{
    extern const ClassInfo ErrorCI;
    extern const ClassInfo DefinitionErrorCI;
    extern const ClassInfo EvalErrorCI;
    extern const ClassInfo RangeErrorCI;
    extern const ClassInfo ReferenceErrorCI;
    extern const ClassInfo SecurityErrorCI;
    extern const ClassInfo SyntaxErrorCI;
    extern const ClassInfo TypeErrorCI;
    extern const ClassInfo URIErrorCI;
    extern const ClassInfo VerifyErrorCI;
    extern const ClassInfo UninitializedErrorCI;
    extern const ClassInfo ArgumentErrorCI;

    extern const ClassInfo DateCI;
    extern const ClassInfo MathCI;
    extern const ClassInfo RegExpCI;
}

namespace fl_display
{
    extern const ClassInfo ActionScriptVersionCI;
    extern const ClassInfo AVM1MovieCI;
    extern const ClassInfo BitmapCI;
    extern const ClassInfo BitmapDataCI;
    extern const ClassInfo BitmapDataChannelCI;
    extern const ClassInfo BlendModeCI;
    extern const ClassInfo CapsStyleCI;
    extern const ClassInfo DisplayObjectCI;
    extern const ClassInfo DisplayObjectContainerCI;
    extern const ClassInfo FrameLabelCI;
    extern const ClassInfo GradientTypeCI;
    extern const ClassInfo GraphicsCI;
    extern const ClassInfo GraphicsBitmapFillCI;
    extern const ClassInfo GraphicsEndFillCI;
    extern const ClassInfo GraphicsGradientFillCI;
    extern const ClassInfo GraphicsPathCI;
    extern const ClassInfo GraphicsPathCommandCI;
    extern const ClassInfo GraphicsSolidFillCI;
    extern const ClassInfo GraphicsStrokeCI;
    extern const ClassInfo GraphicsPathWindingCI;
    extern const ClassInfo IBitmapDrawableCI;
    extern const ClassInfo IGraphicsFillCI;
    extern const ClassInfo IGraphicsPathCI;
    extern const ClassInfo IGraphicsDataCI;
    extern const ClassInfo InteractiveObjectCI;
    extern const ClassInfo InterpolationMethodCI;
    extern const ClassInfo JointStyleCI;
    extern const ClassInfo LineScaleModeCI;
    extern const ClassInfo LoaderCI;
    extern const ClassInfo LoaderInfoCI;
    extern const ClassInfo MorphShapeCI;
    extern const ClassInfo MovieClipCI;
    extern const ClassInfo PixelSnappingCI;
    extern const ClassInfo SceneCI;
    extern const ClassInfo ShapeCI;
    extern const ClassInfo SimpleButtonCI;
    extern const ClassInfo SpreadMethodCI;
    extern const ClassInfo SpriteCI;
    extern const ClassInfo StageCI;
    extern const ClassInfo StageAlignCI;
    extern const ClassInfo StageDisplayStateCI;
    extern const ClassInfo StageQualityCI;
    extern const ClassInfo StageScaleModeCI;
    extern const ClassInfo SWFVersionCI;
    extern const ClassInfo ShaderDataCI;
    extern const ClassInfo ShaderCI;
}

namespace fl_errors
{
    extern const ClassInfo IllegalOperationErrorCI;
    extern const ClassInfo InvalidSWFErrorCI;
    extern const ClassInfo IOErrorCI;
    extern const ClassInfo EOFErrorCI;
    extern const ClassInfo MemoryErrorCI;
    extern const ClassInfo ScriptTimeoutErrorCI;
    extern const ClassInfo StackOverflowErrorCI;
}

namespace fl_events
{
    extern const ClassInfo AccelerometerEventCI;
    extern const ClassInfo ActivityEventCI;
    extern const ClassInfo AsyncErrorEventCI;
    extern const ClassInfo ContextMenuEventCI;
    extern const ClassInfo DataEventCI;
    extern const ClassInfo ErrorEventCI;
    extern const ClassInfo EventCI;
    extern const ClassInfo EventDispatcherCI;
    extern const ClassInfo EventPhaseCI;
    extern const ClassInfo FocusEventCI;
    extern const ClassInfo FullScreenEventCI;
    extern const ClassInfo GeolocationEventCI;
    extern const ClassInfo HTTPStatusEventCI;
    extern const ClassInfo IEventDispatcherCI;
    extern const ClassInfo IOErrorEventCI;
    extern const ClassInfo KeyboardEventCI;
    extern const ClassInfo MouseEventCI;
    extern const ClassInfo NetStatusEventCI;
    extern const ClassInfo ProgressEventCI;
	extern const ClassInfo OutputProgressEventCI;
    extern const ClassInfo SecurityErrorEventCI;
    extern const ClassInfo StatusEventCI;
    extern const ClassInfo SyncEventCI;
    extern const ClassInfo TextEventCI;
    extern const ClassInfo TimerEventCI;
    extern const ClassInfo GestureEventCI;
    extern const ClassInfo TransformGestureEventCI;
    extern const ClassInfo PressAndTapGestureEventCI;
    extern const ClassInfo TouchEventCI;
    extern const ClassInfo GesturePhaseCI;
    extern const ClassInfo AppLifecycleEventCI;
    extern const ClassInfo StageOrientationEventCI;
    extern const ClassInfo StageOrientationCI;
}

namespace fl_external
{
    extern const ClassInfo ExternalInterfaceCI;
    extern const ClassInfo ExtensionContextCI;
}

namespace fl_filters
{
    extern const ClassInfo BevelFilterCI;
    extern const ClassInfo BitmapFilterCI;
    extern const ClassInfo BitmapFilterQualityCI;
    extern const ClassInfo BitmapFilterTypeCI;
    extern const ClassInfo BlurFilterCI;
    extern const ClassInfo ColorMatrixFilterCI;
    extern const ClassInfo ConvolutionFilterCI;
    extern const ClassInfo DisplacementMapFilterCI;
    extern const ClassInfo DisplacementMapFilterModeCI;
    extern const ClassInfo DropShadowFilterCI;
    extern const ClassInfo GlowFilterCI;
    extern const ClassInfo GradientBevelFilterCI;
    extern const ClassInfo GradientGlowFilterCI;
    extern const ClassInfo ShaderFilterCI;
}

namespace fl_geom
{
    extern const ClassInfo ColorTransformCI;
    extern const ClassInfo Vector3DCI;
    extern const ClassInfo Matrix3DCI;
    extern const ClassInfo MatrixCI;
    extern const ClassInfo PerspectiveProjectionCI;
    extern const ClassInfo PointCI;
    extern const ClassInfo RectangleCI;
    extern const ClassInfo TransformCI;
}

namespace fl_net
{
    extern const ClassInfo URLRequestCI;
}

namespace fl_system
{
    extern const ClassInfo ApplicationDomainCI;
    extern const ClassInfo DomainCI;
    extern const ClassInfo CapabilitiesCI;
    extern const ClassInfo LoaderContextCI;
    extern const ClassInfo SecurityCI;
    extern const ClassInfo SecurityDomainCI;
    extern const ClassInfo SecurityPanelCI;
    extern const ClassInfo SystemCI;
}

namespace fl_text
{
    extern const ClassInfo AntiAliasTypeCI;
    extern const ClassInfo CSMSettingsCI;
    extern const ClassInfo FontCI;
    extern const ClassInfo FontStyleCI;
    extern const ClassInfo FontTypeCI;
    extern const ClassInfo GridFitTypeCI;
    extern const ClassInfo StaticTextCI;
    extern const ClassInfo StyleSheetCI;
    extern const ClassInfo TextColorTypeCI;
    extern const ClassInfo TextDisplayModeCI;
    extern const ClassInfo TextFieldCI;
    extern const ClassInfo TextFieldAutoSizeCI;
    extern const ClassInfo TextFieldTypeCI;
    extern const ClassInfo TextFormatCI;
    extern const ClassInfo TextFormatAlignCI;
    extern const ClassInfo TextLineMetricsCI;
    extern const ClassInfo TextRendererCI;
    extern const ClassInfo TextSnapshotCI;
}

namespace fl_ui
{
    extern const ClassInfo ContextMenuBuiltInItemsCI;
    extern const ClassInfo ContextMenuClipboardItemsCI;
    extern const ClassInfo ContextMenuItemCI;
    extern const ClassInfo ContextMenuCI;
    extern const ClassInfo KeyboardCI;
    extern const ClassInfo KeyLocationCI;
    extern const ClassInfo MouseCI;
    extern const ClassInfo MouseCursorCI;
}

namespace fl_utils
{
    extern const ClassInfo ByteArrayCI;
    extern const ClassInfo DictionaryCI;
    extern const ClassInfo EndianCI;
    extern const ClassInfo IDataInputCI;
    extern const ClassInfo IDataOutputCI;
    extern const ClassInfo ProxyCI;
    extern const ClassInfo TimerCI;
}

namespace fl_gfx
{
    extern const ClassInfo SystemExCI;
    extern const ClassInfo FocusManagerCI;
    extern const ClassInfo ExtensionsCI;
    extern const ClassInfo MouseCursorEventCI;
    extern const ClassInfo DisplayObjectExCI;
    extern const ClassInfo InteractiveObjectExCI;
    extern const ClassInfo TextFieldExCI;
    extern const ClassInfo MouseEventExCI;
    extern const ClassInfo KeyboardEventExCI;
    extern const ClassInfo GamePadCI;
    extern const ClassInfo GamePadAnalogEventCI;
    extern const ClassInfo FocusEventExCI;
    extern const ClassInfo TextEventExCI;
    extern const ClassInfo IMEEventExCI;
    extern const ClassInfo IMEExCI;
    extern const ClassInfo IMECandidateListStyleCI;
}

namespace fl
{
    extern const ClassInfo XMLCI;
    extern const ClassInfo XMLListCI;
    extern const ClassInfo QNameCI;
}

namespace fl_net
{
    extern const ClassInfo XMLSocketCI;
}

namespace fl_security
{
    extern const ClassInfo XMLSignatureValidatorCI;
}

namespace fl_xml
{
    extern const ClassInfo XMLDocumentCI;
    extern const ClassInfo XMLNodeCI;
    extern const ClassInfo XMLNodeTypeCI;
}

namespace fl_desktop
{
    extern const ClassInfo ClipboardCI;
    extern const ClassInfo ClipboardFormatsCI;
    extern const ClassInfo ClipboardTransferModeCI;
    extern const ClassInfo IconCI;
    extern const ClassInfo InteractiveIconCI;
    extern const ClassInfo DockIconCI;
    extern const ClassInfo NativeApplicationCI;
    extern const ClassInfo NativeDragActionsCI;
    extern const ClassInfo NativeDragManagerCI;
    extern const ClassInfo NativeDragOptionsCI;
    extern const ClassInfo NotificationTypeCI;
    extern const ClassInfo SystemTrayIconCI;
    extern const ClassInfo UpdaterCI;
}

namespace fl_display
{
    extern const ClassInfo FocusDirectionCI;
    extern const ClassInfo NativeMenuCI;
    extern const ClassInfo NativeMenuItemCI;
    extern const ClassInfo NativeWindowCI;
    extern const ClassInfo NativeWindowDisplayStateCI;
    extern const ClassInfo NativeWindowInitOptionsCI;
    extern const ClassInfo NativeWindowResizeCI;
    extern const ClassInfo NativeWindowSystemChromeCI;
    extern const ClassInfo NativeWindowTypeCI;
    extern const ClassInfo ScreenCI;
}

namespace fl_events
{
    extern const ClassInfo BrowserInvokeEventCI;
    extern const ClassInfo DRMAuthenticateEventCI;
    extern const ClassInfo DRMErrorEventCI;
    extern const ClassInfo DRMStatusEventCI;
    extern const ClassInfo FileListEventCI;
    extern const ClassInfo HTMLUncaughtScriptExceptionEventCI;
    extern const ClassInfo InvokeEventCI;
    extern const ClassInfo NativeDragEventCI;
    extern const ClassInfo NativeWindowBoundsEventCI;
    extern const ClassInfo NativeWindowDisplayStateEventCI;
    extern const ClassInfo ScreenMouseEventCI;
}

namespace fl_filesystem
{
    extern const ClassInfo FileCI;
    extern const ClassInfo FileModeCI;
    extern const ClassInfo FileStreamCI;
}

namespace fl_html
{
    extern const ClassInfo HTMLHistoryItemCI;
    extern const ClassInfo HTMLHostCI;
    extern const ClassInfo HTMLLoaderCI;
    extern const ClassInfo HTMLPDFCapabilityCI;
    extern const ClassInfo HTMLWindowCreateOptionsCI;
}

namespace fl_net
{
    extern const ClassInfo URLRequestDefaultsCI;
}

namespace fl_sensors
{
    extern const ClassInfo AccelerometerCI;
    extern const ClassInfo GeolocationCI;
}

namespace fl_printing
{
    extern const ClassInfo PrintJobCI;
    extern const ClassInfo PrintJobOptionsCI;
    extern const ClassInfo PrintJobOrientationCI;
}

namespace fl_sampler
{
    extern const ClassInfo DeleteObjectSampleCI;
    extern const ClassInfo NewObjectSampleCI;
    extern const ClassInfo SampleCI;
    extern const ClassInfo StackFrameCI;
}

namespace fl_security
{
    extern const ClassInfo IURIDereferencerCI;
    extern const ClassInfo ReferencesValidationSettingCI;
    extern const ClassInfo RevocationCheckSettingsCI;
    extern const ClassInfo SignatureStatusCI;
    extern const ClassInfo SignerTrustSettingsCI;
}

namespace fl_utils
{
    extern const ClassInfo CompressionAlgorithmCI;
}

namespace fl_media
{
    extern const ClassInfo ID3InfoCI;
    extern const ClassInfo MicrophoneCI;
    extern const ClassInfo SoundCI;
    extern const ClassInfo SoundChannelCI;
    extern const ClassInfo SoundLoaderContextCI;
    extern const ClassInfo SoundMixerCI;
    extern const ClassInfo SoundTransformCI;
}

namespace fl_events
{
    extern const ClassInfo IMEEventCI;
}

namespace fl_system
{
    extern const ClassInfo IMECI;
    extern const ClassInfo IMEConversionModeCI;
}

namespace fl_media
{
    extern const ClassInfo CameraCI;
}

namespace fl_net
{
    extern const ClassInfo FileFilterCI;
    extern const ClassInfo FileReferenceCI;
    extern const ClassInfo FileReferenceListCI;
    extern const ClassInfo IDynamicPropertyOutputCI;
    extern const ClassInfo IDynamicPropertyWriterCI;
    extern const ClassInfo LocalConnectionCI;
    extern const ClassInfo NetConnectionCI;
    extern const ClassInfo ObjectEncodingCI;
    extern const ClassInfo ResponderCI;
    extern const ClassInfo SharedObjectCI;
    extern const ClassInfo SharedObjectFlushStatusCI;
    extern const ClassInfo SocketCI;
    extern const ClassInfo URLLoaderCI;
    extern const ClassInfo URLLoaderDataFormatCI;
    extern const ClassInfo URLRequestHeaderCI;
    extern const ClassInfo URLRequestMethodCI;
    extern const ClassInfo URLStreamCI;
    extern const ClassInfo URLVariablesCI;
}

namespace fl_ui
{
    extern const ClassInfo MultitouchCI;
    extern const ClassInfo MultitouchInputModeCI;
}

namespace fl_utils
{
    extern const ClassInfo IExternalizableCI;
}

    
namespace Classes
{
    ///////////////////////////////////////////////////////////////////////////
    // Forward declaration.
    const ClassInfo* ClassRegistrationTable[] =
    {
//##markup##"obj_global_cpp$inheritance_table"       
//##begin##"obj_global_cpp$inheritance_table"

#ifndef SF_AS3_NO_FLASH
        &AS3::fl_accessibility::ISearchableTextCI,
        &AS3::fl_accessibility::ISimpleTextSelectionCI,
        &AS3::fl_accessibility::AccessibilityCI,
        &AS3::fl_accessibility::AccessibilityImplementationCI,
        &AS3::fl_accessibility::AccessibilityPropertiesCI,
        &AS3::fl::ErrorCI,
        &AS3::fl::DefinitionErrorCI,
        &AS3::fl::EvalErrorCI,
        &AS3::fl::RangeErrorCI,
        &AS3::fl::ReferenceErrorCI,
        &AS3::fl::SecurityErrorCI,
        &AS3::fl::SyntaxErrorCI,
        &AS3::fl::TypeErrorCI,
        &AS3::fl::URIErrorCI,
        &AS3::fl::VerifyErrorCI,
        &AS3::fl::UninitializedErrorCI,
        &AS3::fl::ArgumentErrorCI,
        &AS3::fl::DateCI,
        &AS3::fl::MathCI,
        &AS3::fl::RegExpCI,
        &AS3::fl::DomainCI,
        &AS3::fl_display::ActionScriptVersionCI,
        &AS3::fl_display::AVM1MovieCI,
        &AS3::fl_display::BitmapCI,
        &AS3::fl_display::BitmapDataCI,
        &AS3::fl_display::BitmapDataChannelCI,
        &AS3::fl_display::BlendModeCI,
        &AS3::fl_display::CapsStyleCI,
        &AS3::fl_display::DisplayObjectCI,
        &AS3::fl_display::DisplayObjectContainerCI,
        &AS3::fl_display::FrameLabelCI,
        &AS3::fl_display::GradientTypeCI,
        &AS3::fl_display::GraphicsCI,
        &AS3::fl_display::IBitmapDrawableCI,
        &AS3::fl_display::IGraphicsFillCI,
        &AS3::fl_display::IGraphicsPathCI,
        &AS3::fl_display::IGraphicsDataCI,
        &AS3::fl_display::GraphicsBitmapFillCI,
        &AS3::fl_display::GraphicsEndFillCI,
        &AS3::fl_display::GraphicsGradientFillCI,
        &AS3::fl_display::GraphicsPathCI,
        &AS3::fl_display::GraphicsPathCommandCI,
        &AS3::fl_display::GraphicsPathWindingCI,
        &AS3::fl_display::GraphicsSolidFillCI,
        &AS3::fl_display::GraphicsStrokeCI,
        &AS3::fl_display::InteractiveObjectCI,
        &AS3::fl_display::InterpolationMethodCI,
        &AS3::fl_display::JointStyleCI,
        &AS3::fl_display::LineScaleModeCI,
        &AS3::fl_display::LoaderCI,
        &AS3::fl_display::LoaderInfoCI,
        &AS3::fl_display::MorphShapeCI,
        &AS3::fl_display::MovieClipCI,
        &AS3::fl_display::PixelSnappingCI,
        &AS3::fl_display::SceneCI,
        &AS3::fl_display::ShapeCI,
        &AS3::fl_display::SimpleButtonCI,
        &AS3::fl_display::SpreadMethodCI,
        &AS3::fl_display::SpriteCI,
        &AS3::fl_display::StageCI,
        &AS3::fl_display::StageAlignCI,
        &AS3::fl_display::StageDisplayStateCI,
        &AS3::fl_display::StageQualityCI,
        &AS3::fl_display::StageScaleModeCI,
        &AS3::fl_display::SWFVersionCI,
        &AS3::fl_display::ShaderDataCI,
        &AS3::fl_display::ShaderCI,
        &AS3::fl_errors::IllegalOperationErrorCI,
        &AS3::fl_errors::InvalidSWFErrorCI,
        &AS3::fl_errors::IOErrorCI,
        &AS3::fl_errors::EOFErrorCI,
        &AS3::fl_errors::MemoryErrorCI,
        &AS3::fl_errors::ScriptTimeoutErrorCI,
        &AS3::fl_errors::StackOverflowErrorCI,
        &AS3::fl_events::AccelerometerEventCI,
        &AS3::fl_events::ActivityEventCI,
        &AS3::fl_events::AsyncErrorEventCI,
        &AS3::fl_events::ContextMenuEventCI,
        &AS3::fl_events::DataEventCI,
        &AS3::fl_events::ErrorEventCI,
        &AS3::fl_events::EventCI,
        &AS3::fl_events::EventDispatcherCI,
        &AS3::fl_events::EventPhaseCI,
        &AS3::fl_events::FocusEventCI,
        &AS3::fl_events::FullScreenEventCI,
        &AS3::fl_events::GeolocationEventCI,
        &AS3::fl_events::HTTPStatusEventCI,
        &AS3::fl_events::IEventDispatcherCI,
        &AS3::fl_events::IOErrorEventCI,
        &AS3::fl_events::KeyboardEventCI,
        &AS3::fl_events::MouseEventCI,
        &AS3::fl_events::NetStatusEventCI,
        &AS3::fl_events::ProgressEventCI,
        &AS3::fl_events::SecurityErrorEventCI,
        &AS3::fl_events::StatusEventCI,
        &AS3::fl_events::SyncEventCI,
        &AS3::fl_events::TextEventCI,
        &AS3::fl_events::TimerEventCI,
        &AS3::fl_events::GestureEventCI,
        &AS3::fl_events::TransformGestureEventCI,
        &AS3::fl_events::PressAndTapGestureEventCI,
        &AS3::fl_events::TouchEventCI,
        &AS3::fl_events::GesturePhaseCI,
        &AS3::fl_events::AppLifecycleEventCI,
        &AS3::fl_events::StageOrientationEventCI,
        &AS3::fl_events::StageOrientationCI,
        &AS3::fl_external::ExternalInterfaceCI,
        &AS3::fl_filters::BevelFilterCI,
        &AS3::fl_filters::BitmapFilterCI,
        &AS3::fl_filters::BitmapFilterQualityCI,
        &AS3::fl_filters::BitmapFilterTypeCI,
        &AS3::fl_filters::BlurFilterCI,
        &AS3::fl_filters::ColorMatrixFilterCI,
        &AS3::fl_filters::ConvolutionFilterCI,
        &AS3::fl_filters::DisplacementMapFilterCI,
        &AS3::fl_filters::DisplacementMapFilterModeCI,
        &AS3::fl_filters::DropShadowFilterCI,
        &AS3::fl_filters::GlowFilterCI,
        &AS3::fl_filters::GradientBevelFilterCI,
        &AS3::fl_filters::GradientGlowFilterCI,
        &AS3::fl_filters::ShaderFilterCI,
        &AS3::fl_geom::ColorTransformCI,
        &AS3::fl_geom::Vector3DCI,
        &AS3::fl_geom::Matrix3DCI,
        &AS3::fl_geom::MatrixCI,
        &AS3::fl_geom::PerspectiveProjectionCI,
        &AS3::fl_geom::PointCI,
        &AS3::fl_geom::RectangleCI,
        &AS3::fl_geom::TransformCI,
        &AS3::fl_net::URLRequestCI,
        &AS3::fl_sensors::AccelerometerCI,
        &AS3::fl_system::ApplicationDomainCI,
        &AS3::fl_system::CapabilitiesCI,
        &AS3::fl_system::LoaderContextCI,
        &AS3::fl_system::SecurityCI,
        &AS3::fl_system::SecurityDomainCI,
        &AS3::fl_system::SecurityPanelCI,
        &AS3::fl_system::SystemCI,
        &AS3::fl_text::AntiAliasTypeCI,
        &AS3::fl_text::CSMSettingsCI,
        &AS3::fl_text::FontCI,
        &AS3::fl_text::FontStyleCI,
        &AS3::fl_text::FontTypeCI,
        &AS3::fl_text::GridFitTypeCI,
        &AS3::fl_text::StaticTextCI,
        &AS3::fl_text::StyleSheetCI,
        &AS3::fl_text::TextColorTypeCI,
        &AS3::fl_text::TextDisplayModeCI,
        &AS3::fl_text::TextFieldCI,
        &AS3::fl_text::TextFieldAutoSizeCI,
        &AS3::fl_text::TextFieldTypeCI,
        &AS3::fl_text::TextFormatCI,
        &AS3::fl_text::TextFormatAlignCI,
        &AS3::fl_text::TextLineMetricsCI,
        &AS3::fl_text::TextRendererCI,
        &AS3::fl_text::TextSnapshotCI,
        &AS3::fl_ui::ContextMenuBuiltInItemsCI,
        &AS3::fl_ui::ContextMenuClipboardItemsCI,
        &AS3::fl_ui::ContextMenuItemCI,
        &AS3::fl_ui::ContextMenuCI,
        &AS3::fl_ui::KeyboardCI,
        &AS3::fl_ui::KeyLocationCI,
        &AS3::fl_ui::MouseCI,
        &AS3::fl_ui::MouseCursorCI,
        &AS3::fl_utils::ByteArrayCI,
        &AS3::fl_utils::DictionaryCI,
        &AS3::fl_utils::EndianCI,
        &AS3::fl_utils::IDataInputCI,
        &AS3::fl_utils::IDataOutputCI,
        &AS3::fl_utils::ProxyCI,
        &AS3::fl_utils::TimerCI,
        &AS3::fl_gfx::SystemExCI,
        &AS3::fl_gfx::FocusManagerCI,
        &AS3::fl_gfx::ExtensionsCI,
        &AS3::fl_gfx::MouseCursorEventCI,
        &AS3::fl_gfx::DisplayObjectExCI,
        &AS3::fl_gfx::InteractiveObjectExCI,
        &AS3::fl_gfx::TextFieldExCI,
        &AS3::fl_gfx::MouseEventExCI,
        &AS3::fl_gfx::KeyboardEventExCI,
        &AS3::fl_gfx::GamePadCI,
        &AS3::fl_gfx::GamePadAnalogEventCI,
        &AS3::fl_gfx::FocusEventExCI,
        &AS3::fl_gfx::TextEventExCI,
        &AS3::fl_gfx::IMEEventExCI,
        &AS3::fl_gfx::IMEExCI,
        &AS3::fl_gfx::IMECandidateListStyleCI,
#ifdef SF_AS3_VERSION_XML
        &AS3::fl::XMLCI,
        &AS3::fl::XMLListCI,
        &AS3::fl::QNameCI,
        &AS3::fl_net::XMLSocketCI,
        &AS3::fl_xml::XMLDocumentCI,
        &AS3::fl_xml::XMLNodeCI,
        &AS3::fl_xml::XMLNodeTypeCI,
#endif // SF_AS3_VERSION_XML
#ifdef SF_AS3_VERSION_AIR
        &AS3::fl_desktop::ClipboardCI,
        &AS3::fl_desktop::ClipboardFormatsCI,
        &AS3::fl_desktop::ClipboardTransferModeCI,
        &AS3::fl_desktop::IconCI,
        &AS3::fl_desktop::InteractiveIconCI,
        &AS3::fl_desktop::DockIconCI,
        &AS3::fl_desktop::NativeApplicationCI,
        &AS3::fl_desktop::NativeDragActionsCI,
        &AS3::fl_desktop::NativeDragManagerCI,
        &AS3::fl_desktop::NativeDragOptionsCI,
        &AS3::fl_desktop::NotificationTypeCI,
        &AS3::fl_desktop::SystemTrayIconCI,
        &AS3::fl_desktop::UpdaterCI,
        &AS3::fl_display::FocusDirectionCI,
        &AS3::fl_display::NativeMenuCI,
        &AS3::fl_display::NativeMenuItemCI,
        &AS3::fl_display::NativeWindowCI,
        &AS3::fl_display::NativeWindowDisplayStateCI,
        &AS3::fl_display::NativeWindowInitOptionsCI,
        &AS3::fl_display::NativeWindowResizeCI,
        &AS3::fl_display::NativeWindowSystemChromeCI,
        &AS3::fl_display::NativeWindowTypeCI,
        &AS3::fl_display::ScreenCI,
        &AS3::fl_events::BrowserInvokeEventCI,
        &AS3::fl_events::DRMAuthenticateEventCI,
        &AS3::fl_events::DRMErrorEventCI,
        &AS3::fl_events::DRMStatusEventCI,
        &AS3::fl_events::FileListEventCI,
        &AS3::fl_events::HTMLUncaughtScriptExceptionEventCI,
        &AS3::fl_events::InvokeEventCI,
        &AS3::fl_events::NativeDragEventCI,
        &AS3::fl_events::NativeWindowBoundsEventCI,
        &AS3::fl_events::NativeWindowDisplayStateEventCI,
        &AS3::fl_events::OutputProgressEventCI,
        &AS3::fl_events::ScreenMouseEventCI,
        &AS3::fl_filesystem::FileCI,
        &AS3::fl_filesystem::FileModeCI,
        &AS3::fl_filesystem::FileStreamCI,
        &AS3::fl_html::HTMLHistoryItemCI,
        &AS3::fl_html::HTMLHostCI,
        &AS3::fl_html::HTMLLoaderCI,
        &AS3::fl_html::HTMLPDFCapabilityCI,
        &AS3::fl_html::HTMLWindowCreateOptionsCI,
        &AS3::fl_net::URLRequestDefaultsCI,
        &AS3::fl_printing::PrintJobCI,
        &AS3::fl_printing::PrintJobOptionsCI,
        &AS3::fl_printing::PrintJobOrientationCI,
        &AS3::fl_sampler::DeleteObjectSampleCI,
        &AS3::fl_sampler::NewObjectSampleCI,
        &AS3::fl_sampler::SampleCI,
        &AS3::fl_sampler::StackFrameCI,
        &AS3::fl_security::IURIDereferencerCI,
        &AS3::fl_security::ReferencesValidationSettingCI,
        &AS3::fl_security::RevocationCheckSettingsCI,
        &AS3::fl_security::SignatureStatusCI,
        &AS3::fl_security::SignerTrustSettingsCI,
        &AS3::fl_security::XMLSignatureValidatorCI,
        &AS3::fl_sensors::GeolocationCI,
        &AS3::fl_utils::CompressionAlgorithmCI,
#endif // SF_AS3_VERSION_AIR
#ifdef SF_AS3_VERSION_SOUND
        &AS3::fl_media::ID3InfoCI,
        &AS3::fl_media::MicrophoneCI,
        &AS3::fl_media::SoundCI,
        &AS3::fl_media::SoundChannelCI,
        &AS3::fl_media::SoundLoaderContextCI,
        &AS3::fl_media::SoundMixerCI,
        &AS3::fl_media::SoundTransformCI,
#endif // SF_AS3_VERSION_SOUND
#ifdef SF_AS3_VERSION_IME
        &AS3::fl_events::IMEEventCI,
        &AS3::fl_system::IMECI,
        &AS3::fl_system::IMEConversionModeCI,
#endif // SF_AS3_VERSION_IME
#ifdef SF_AS3_VERSION_EXTENSION_CONTEXT
        &AS3::fl_external::ExtensionContextCI,
#endif // SF_AS3_VERSION_EXTENSION_CONTEXT
#ifdef SF_AS3_VERSION_VIDEO
        &AS3::fl_media::CameraCI,
#endif // SF_AS3_VERSION_VIDEO
#ifdef SF_AS3_VERSION_NET
        &AS3::fl_net::FileFilterCI,
        &AS3::fl_net::FileReferenceCI,
        &AS3::fl_net::FileReferenceListCI,
        &AS3::fl_net::IDynamicPropertyOutputCI,
        &AS3::fl_net::IDynamicPropertyWriterCI,
        &AS3::fl_net::LocalConnectionCI,
        &AS3::fl_net::NetConnectionCI,
        &AS3::fl_net::ObjectEncodingCI,
        &AS3::fl_net::ResponderCI,
        &AS3::fl_net::SocketCI,
        &AS3::fl_net::URLLoaderCI,
        &AS3::fl_net::URLLoaderDataFormatCI,
        &AS3::fl_net::URLRequestHeaderCI,
        &AS3::fl_net::URLRequestMethodCI,
        &AS3::fl_net::URLStreamCI,
        &AS3::fl_net::URLVariablesCI,
#endif // SF_AS3_VERSION_NET
#ifdef SF_AS3_VERSION_SHARED_OBJECT
        &AS3::fl_net::SharedObjectCI,
        &AS3::fl_net::SharedObjectFlushStatusCI,
#endif // SF_AS3_VERSION_SHARED_OBJECT
#ifdef SF_AS3_VERSION_MULTITOUCH
        &AS3::fl_ui::MultitouchCI,
        &AS3::fl_ui::MultitouchInputModeCI,
#endif // SF_AS3_VERSION_MULTITOUCH
#ifdef SF_AS3_VERSION_REST
        &AS3::fl_utils::IExternalizableCI,
#endif // SF_AS3_VERSION_REST
#endif

//##end##"obj_global_cpp$inheritance_table"
        NULL
    };
} // namespace Classes

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif // GFX_AS3_SUPPORT

#endif
