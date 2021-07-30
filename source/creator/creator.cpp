/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Copyright 2021, Wabi.
 */

/**
 * @file
 * Creator.
 * Creating Chaos.
 */

#include <wabi/base/arch/defines.h>

#if defined(ARCH_OS_WINDOWS)

#  include <Windows.h>
#  include "KKE_precomp.h"

/**
 * The official Windows Runtime. Kraken
 * supports the Windows platform as a
 * first class citizen.
 * 
 * Copyright (C) Microsoft Corporation. */

/*  Windows :: Base */
#  include <winrt/base.h>

/*  Windows :: Artifical Intelligence */
#include <Windows.Ai.MachineLearning.h>
#include <Windows.Ai.MachineLearning.Preview.h>

/*  Windows :: ApplicationModel */
#include <Windows.ApplicationModel.h>
#include <Windows.ApplicationModel.Activation.h>
#include <Windows.ApplicationModel.Appextensions.h>
#include <Windows.ApplicationModel.Appointments.AppointmentsProvider.h>
#include <Windows.ApplicationModel.Appointments.DataProvider.h>
#include <Windows.ApplicationModel.Appointments.h>
#include <Windows.ApplicationModel.Appservice.h>
#include <Windows.ApplicationModel.Background.h>
#include <Windows.ApplicationModel.Calls.Background.h>
#include <Windows.ApplicationModel.Calls.h>
#include <Windows.ApplicationModel.Calls.Provider.h>
#include <Windows.ApplicationModel.Chat.h>
#include <Windows.ApplicationModel.CommunicationBlocking.h>
#include <Windows.ApplicationModel.Contacts.DataProvider.h>
#include <Windows.ApplicationModel.Contacts.h>
#include <Windows.ApplicationModel.Contacts.Provider.h>
#include <Windows.ApplicationModel.ConversationalAgent.h>
#include <Windows.ApplicationModel.Core.h>
#include <Windows.ApplicationModel.Datatransfer.DragDrop.Core.h>
#include <Windows.ApplicationModel.Datatransfer.Dragdrop.h>
#include <Windows.ApplicationModel.DataTransfer.h>
#include <Windows.ApplicationModel.DataTransfer.ShareTarget.h>
#include <Windows.ApplicationModel.Email.DataProvider.h>
#include <Windows.ApplicationModel.Email.h>
#include <Windows.ApplicationModel.Extendedexecution.Foreground.h>
#include <Windows.ApplicationModel.Extendedexecution.h>
#include <Windows.ApplicationModel.Holographic.h>
#include <Windows.ApplicationModel.Lockscreen.h>
#include <Windows.ApplicationModel.Payments.h>
#include <Windows.ApplicationModel.Payments.Provider.h>
#include <Windows.ApplicationModel.Preview.Holographic.h>
#include <Windows.ApplicationModel.Preview.InkWorkSpace.h>
#include <Windows.ApplicationModel.Preview.Notes.h>
#include <Windows.ApplicationModel.Resources.Core.h>
#include <Windows.ApplicationModel.Resources.h>
#include <Windows.ApplicationModel.Resources.Management.h>
#include <Windows.ApplicationModel.Search.Core.h>
#include <Windows.ApplicationModel.Search.h>
#include <Windows.ApplicationModel.SocialInfo.h>
#include <Windows.ApplicationModel.SocialInfo.Provider.h>
#include <Windows.ApplicationModel.Store.h>
#include <Windows.ApplicationModel.Store.LicenseManagement.h>
#include <Windows.ApplicationModel.Store.Preview.h>
#include <Windows.ApplicationModel.Store.Preview.InstallControl.h>
#include <Windows.ApplicationModel.Useractivities.Core.h>
#include <Windows.ApplicationModel.Useractivities.h>
#include <Windows.ApplicationModel.Userdataaccounts.h>
#include <Windows.ApplicationModel.Userdataaccounts.Provider.h>
#include <Windows.ApplicationModel.Userdataaccounts.Systemaccess.h>
#include <Windows.ApplicationModel.Userdatatasks.Dataprovider.h>
#include <Windows.ApplicationModel.Userdatatasks.h>
#include <Windows.ApplicationModel.Voicecommands.h>
#include <Windows.ApplicationModel.Wallet.h>
#include <Windows.ApplicationModel.Wallet.System.h>

/*  Windows :: Data */
#include <Windows.Data.Html.h>
#include <Windows.Data.Json.h>
#include <Windows.Data.Pdf.h>
#include <Windows.Data.Text.h>
#include <Windows.Data.Xml.Dom.h>
#include <Windows.Data.Xml.Xsl.h>

/*  Windows :: Devices */
#include <Windows.Devices.h>
#include <Windows.Devices.Adc.h>
#include <Windows.Devices.Adc.Provider.h>
#include <Windows.Devices.AllJoyN.h>
#include <Windows.Devices.Background.h>
#include <Windows.Devices.Bluetooth.Advertisement.h>
#include <Windows.Devices.Bluetooth.Background.h>
#include <Windows.Devices.Bluetooth.GenericAttributeProfile.h>
#include <Windows.Devices.Bluetooth.h>
#include <Windows.Devices.Bluetooth.RFComm.h>
#include <Windows.Devices.Custom.h>
#include <Windows.Devices.Display.Core.h>
#include <Windows.Devices.Display.h>
#include <Windows.Devices.Enumeration.h>
#include <Windows.Devices.Enumeration.PNP.h>
#include <Windows.Devices.Geolocation.GeoFencing.h>
#include <Windows.Devices.Geolocation.h>
#include <Windows.Devices.Gpio.h>
#include <Windows.Devices.Gpio.Provider.h>
#include <Windows.Devices.Haptics.h>
#include <Windows.Devices.HumanInterfaceDevice.h>
#include <Windows.Devices.I2C.h>
#include <Windows.Devices.I2C.Provider.h>
#include <Windows.Devices.Input.h>
#include <Windows.Devices.Input.Preview.h>
#include <Windows.Devices.Lights.Effects.h>
#include <Windows.Devices.Lights.h>
#include <Windows.Devices.Midi.h>
#include <Windows.Devices.Perception.h>
#include <Windows.Devices.Perception.Provider.h>
#include <Windows.Devices.PointOfService.h>
#include <Windows.Devices.PointOfService.Provider.h>
#include <Windows.Devices.Portable.h>
#include <Windows.Devices.Power.h>
#include <Windows.Devices.Printers.Extensions.h>
#include <Windows.Devices.Printers.h>
#include <Windows.Devices.PWM.h>
#include <Windows.Devices.PWM.Provider.h>
#include <Windows.Devices.Radios.h>
#include <Windows.Devices.Scanners.h>
#include <Windows.Devices.Sensors.custom.h>
#include <Windows.Devices.Sensors.h>
#include <Windows.Devices.SerialCommunication.h>
#include <Windows.Devices.SmartCards.h>
#include <Windows.Devices.SMS.h>
#include <Windows.Devices.SPI.h>
#include <Windows.Devices.SPI.Provider.h>
#include <Windows.Devices.USB.h>
#include <Windows.Devices.WIFI.h>
#include <Windows.Devices.WIFIDirect.h>
#include <Windows.Devices.WIFIDirect.Services.h>

/*  Windows :: Foundation */
#include <Windows.Foundation.h>
#include <Windows.Foundation.Collections.h>
#include <Windows.Foundation.Diagnostics.h>
#include <Windows.Foundation.MetaData.h>
#include <Windows.Foundation.Numerics.h>

/*  Windows :: Gaming */
#include <Windows.Gaming.UI.h>
#include <Windows.Gaming.Input.Custom.h>
#include <Windows.Gaming.Input.ForceFeedback.h>
#include <Windows.Gaming.Input.h>
#include <Windows.Gaming.Input.Preview.h>
#include <Windows.Gaming.Preview.GamesEnumeration.h>
#include <Windows.Gaming.XboxLive.Storage.h>

/*  Windows :: Globalization */
#include <Windows.Globalization.h>
#include <Windows.Globalization.Collation.h>
#include <Windows.Globalization.DateTimeFormatting.h>
#include <Windows.Globalization.Fonts.h>
#include <Windows.Globalization.NumberFormatting.h>
#include <Windows.Globalization.PhoneNumberFormatting.h>

/*  Windows :: Graphics */
#include <Windows.Graphics.h>
#include <Windows.Graphics.Capture.h>
#include <Windows.Graphics.DirectX.Direct3D11.h>
#include <Windows.Graphics.DirectX.h>
#include <Windows.Graphics.Display.Core.h>
#include <Windows.Graphics.Display.h>
#include <Windows.Graphics.Effects.h>
#include <Windows.Graphics.HoloGraphic.h>
#include <Windows.Graphics.Imaging.h>
#include <Windows.Graphics.Printing.h>
#include <Windows.Graphics.Printing.OptionDetails.h>
#include <Windows.Graphics.Printing.PrintSupport.h>
#include <Windows.Graphics.Printing.PrintTicket.h>
#include <Windows.Graphics.Printing.WorkFlow.h>
#include <Windows.Graphics.Printing3D.h>

/*  Windows :: Management */
#include <Windows.Management.h>
#include <Windows.Management.Core.h>
#include <Windows.Management.Deployment.h>
#include <Windows.Management.Deployment.Preview.h>
#include <Windows.Management.Policies.h>
#include <Windows.Management.Update.h>
#include <Windows.Management.Workplace.h>

/*  Windows :: Media */
#include <Windows.Media.h>
#include <Windows.Media.AppBroadcasting.h>
#include <Windows.Media.AppRecording.h>
#include <Windows.Media.Audio.h>
#include <Windows.Media.Capture.Core.h>
#include <Windows.Media.Capture.Frames.h>
#include <Windows.Media.Capture.h>
#include <Windows.Media.Casting.h>
#include <Windows.Media.ClosedCaptioning.h>
#include <Windows.Media.ContentRestrictions.h>
#include <Windows.Media.Control.h>
#include <Windows.Media.Core.h>
#include <Windows.Media.Core.Preview.h>
#include <Windows.Media.Devices.Core.h>
#include <Windows.Media.Devices.h>
#include <Windows.Media.DialProtocol.h>
#include <Windows.Media.Editing.h>
#include <Windows.Media.Effects.h>
#include <Windows.Media.Faceanalysis.h>
#include <Windows.Media.Import.h>
#include <Windows.Media.MediaProperties.h>
#include <Windows.Media.MiraCast.h>
#include <Windows.Media.OCR.h>
#include <Windows.Media.PlayBack.h>
#include <Windows.Media.Playlists.h>
#include <Windows.Media.PlayTo.h>
#include <Windows.Media.Protection.h>
#include <Windows.Media.Protection.PlayReady.h>
#include <Windows.Media.Render.h>
#include <Windows.Media.Speechrecognition.h>
#include <Windows.Media.Speechsynthesis.h>
#include <Windows.Media.Streaming.adaptive.h>
#include <Windows.Media.Transcoding.h>

/*  Windows :: Networking */
#include <Windows.Networking.h>
#include <Windows.Networking.BackgroundTransfer.h>
#include <Windows.Networking.Connectivity.h>
#include <Windows.Networking.NetworkOperators.h>
#include <Windows.Networking.Proximity.h>
#include <Windows.Networking.PushNotifications.h>
#include <Windows.Networking.ServiceDiscovery.DNSSD.h>
#include <Windows.Networking.Sockets.h>
#include <Windows.Networking.VPN.h>
#include <Windows.Networking.XboxLive.h>

/*  Windows :: Perception */
#include <Windows.Perception.h>
#include <Windows.Perception.Automation.Core.h>
#include <Windows.Perception.People.h>
#include <Windows.Perception.Spatial.h>
#include <Windows.Perception.Spatial.Preview.h>
#include <Windows.Perception.Spatial.Surfaces.h>

/*  Windows :: Phone */
#include <Windows.Phone.Networking.VOIP.h>
#include <Windows.Phone.StartScreen.h>
#include <Windows.Phone.UI.Core.h>

/*  Windows :: Security */
#include <Windows.Security.Authentication.Identity.Core.h>
#include <Windows.Security.Authentication.Identity.h>
#include <Windows.Security.Authentication.Identity.Provider.h>
#include <Windows.Security.Authentication.OnlineID.h>
#include <Windows.Security.Authentication.Web.Core.h>
#include <Windows.Security.Authentication.Web.h>
#include <Windows.Security.Authentication.Web.Provider.h>
#include <Windows.Security.Authorization.AppCapabilityAccess.h>
#include <Windows.Security.Credentials.h>
#include <Windows.Security.Credentials.UI.h>
#include <Windows.Security.Cryptography.Certificates.h>
#include <Windows.Security.Cryptography.Core.h>
#include <Windows.Security.Cryptography.Dataprotection.h>
#include <Windows.Security.Cryptography.h>
#include <Windows.Security.DataProtection.h>
#include <Windows.Security.EnterpriseData.h>
#include <Windows.Security.ExchangeActiveSyncProvisioning.h>
#include <Windows.Security.Isolation.h>

/*  Windows :: Services */
#include <Windows.Services.Cortana.h>
#include <Windows.Services.Maps.Guidance.h>
#include <Windows.Services.Maps.h>
#include <Windows.Services.Maps.Localsearch.h>
#include <Windows.Services.Maps.OfflineMaps.h>
#include <Windows.Services.Store.h>
#include <Windows.Services.TargetedContent.h>

/*  Windows :: Storage */
#include <Windows.Storage.h>
#include <Windows.Storage.AccessCache.h>
#include <Windows.Storage.BulkAccess.h>
#include <Windows.Storage.Compression.h>
#include <Windows.Storage.FileProperties.h>
#include <Windows.Storage.Pickers.h>
#include <Windows.Storage.Pickers.Provider.h>
#include <Windows.Storage.Provider.h>
#include <Windows.Storage.Search.h>
#include <Windows.Storage.Streams.h>

/*  Windows :: System */
#include <Windows.System.h>
#include <Windows.System.Diagnostics.DevicePortal.h>
#include <Windows.System.Diagnostics.h>
#include <Windows.System.Diagnostics.Telemetry.h>
#include <Windows.System.Diagnostics.Tracereporting.h>
#include <Windows.System.Display.h>
#include <Windows.System.Implementation.FileExplorer.h>
#include <Windows.System.Inventory.h>
#include <Windows.System.Power.diagnostics.h>
#include <Windows.System.Power.h>
#include <Windows.System.Preview.h>
#include <Windows.System.Profile.h>
#include <Windows.System.Profile.SystemManufacturers.h>
#include <Windows.System.RemoteDesktop.h>
#include <Windows.System.RemoteDesktop.input.h>
#include <Windows.System.RemoteSystems.h>
#include <Windows.System.Threading.Core.h>
#include <Windows.System.Threading.h>
#include <Windows.System.Update.h>
#include <Windows.System.UserProfile.h>

/*  Windows :: UI */
#include <Windows.UI.h>
#include <Windows.UI.Accessibility.h>
#include <Windows.UI.ApplicationSettings.h>
#include <Windows.UI.Composition.Core.h>
#include <Windows.UI.Composition.Desktop.h>
#include <Windows.UI.Composition.Diagnostics.h>
#include <Windows.UI.Composition.Effects.h>
#include <Windows.UI.Composition.h>
#include <Windows.UI.Composition.Interactions.h>
#include <Windows.UI.Composition.Scenes.h>
#include <Windows.UI.Core.AnimationMetrics.h>
#include <Windows.UI.Core.h>
#include <Windows.UI.Core.Preview.h>
#include <Windows.UI.Input.Core.h>
#include <Windows.UI.Input.h>
#include <Windows.UI.Input.Inking.Analysis.h>
#include <Windows.UI.Input.Inking.Core.h>
#include <Windows.UI.Input.Inking.h>
#include <Windows.UI.Input.Inking.Preview.h>
#include <Windows.UI.Input.Preview.h>
#include <Windows.UI.Input.Preview.Injection.h>
#include <Windows.UI.Input.Spatial.h>
#include <Windows.UI.NotifiCations.h>
#include <Windows.UI.NotifiCations.Management.h>
#include <Windows.UI.Popups.h>
#include <Windows.UI.Shell.h>
#include <Windows.UI.Startscreen.h>
#include <Windows.UI.Text.core.h>
#include <Windows.UI.Text.h>
#include <Windows.UI.UIAutomation.Core.h>
#include <Windows.UI.UIAutomation.h>
#include <Windows.UI.ViewManagement.Core.h>
#include <Windows.UI.ViewManagement.h>
#include <Windows.UI.WebUI.h>
#include <Windows.UI.WindowManagement.h>
#include <Windows.UI.WindowManagement.Preview.h>

/*  Windows :: XAML */
#include <Windows.UI.XAML.Automation.h>
#include <Windows.UI.XAML.Automation.Peers.h>
#include <Windows.UI.XAML.Automation.Provider.h>
#include <Windows.UI.XAML.Automation.Text.h>
#include <Windows.UI.XAML.Controls.h>
#include <Windows.UI.XAML.Controls.Maps.h>
#include <Windows.UI.XAML.Controls.Primitives.h>
#include <Windows.UI.XAML.Core.Direct.h>
#include <Windows.UI.XAML.Data.h>
#include <Windows.UI.XAML.Documents.h>
#include <Windows.UI.XAML.h>
#include <Windows.UI.XAML.Hosting.h>
#include <Windows.UI.XAML.Input.h>
#include <Windows.UI.XAML.Interop.h>
#include <Windows.UI.XAML.Markup.h>
#include <Windows.UI.XAML.Media.Animation.h>
#include <Windows.UI.XAML.Media.h>
#include <Windows.UI.XAML.Media.Imaging.h>
#include <Windows.UI.XAML.Media.Media3D.h>
#include <Windows.UI.XAML.Navigation.h>
#include <Windows.UI.XAML.Printing.h>
#include <Windows.UI.XAML.Resources.h>
#include <Windows.UI.XAML.Shapes.h>

/*  Windows :: Web */
#include <Windows.Web.Atompub.h>
#include <Windows.Web.h>
#include <Windows.Web.HTTP.Diagnostics.h>
#include <Windows.Web.HTTP.Filters.h>
#include <Windows.Web.HTTP.h>
#include <Windows.Web.HTTP.Headers.h>
#include <Windows.Web.Syndication.h>
#include <Windows.Web.UI.h>
#include <Windows.Web.UI.Interop.h>

#endif /* ARCH_OS_WINDOWS */

#include "KLI_threads.h"

#include "KKE_appdir.h"
#include "KKE_context.h"
#include "KKE_main.h"

#include "UNI_pixar_utils.h"

#include "WM_api.h"
#include "WM_init_exit.h"
#include "WM_window.h"

#include "creator.h"

#if defined(ARCH_OS_WINDOWS)

namespace MICROSOFT = winrt;

using namespace MICROSOFT::Windows;
using namespace MICROSOFT::Windows::ApplicationModel;
using namespace MICROSOFT::Windows::Data;
using namespace MICROSOFT::Windows::Devices;
using namespace MICROSOFT::Windows::Foundation;
using namespace MICROSOFT::Windows::Graphics;
using namespace MICROSOFT::Windows::Media;
using namespace MICROSOFT::Windows::Networking;
using namespace MICROSOFT::Windows::Security;
using namespace MICROSOFT::Windows::Storage;
using namespace MICROSOFT::Windows::System;
using namespace MICROSOFT::Windows::UI;

#endif /* ARCH_OS_WINDOWS */

WABI_NAMESPACE_USING

#ifdef WIN32
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
#else
int main(int argc, const char **argv)
#endif /* WIN32 */
{
#if defined(ARCH_OS_WINDOWS)
  MICROSOFT::init_apartment();
#endif /* ARCH_OS_WINDOWS */

  kContext *C;

  // /* Environment variables. */
  CREATOR_kraken_env_init();

  // /* Create Context C. */
  C = CTX_create();

  // /* Initialize path to executable. */
  // KKE_appdir_program_path_init();

  // /* Initialize Threads. */
  // KLI_threadapi_init();

  // /* Initialize Globals (paths, sys). */
  // KKE_kraken_globals_init();

  // /* Init plugins. */
  // KKE_kraken_plugins_init();

  /* Init & parse args. */

  // CREATOR_setup_args(argc, (const char **)argv);
  // if (CREATOR_parse_args(argc, (const char **)argv) != 0)
  // {
  //   return 0;
  // }

  // KKE_appdir_init();

  // /* Determining Stage Configuration and Loadup. */
  // KKE_kraken_main_init(C, argc, (const char **)argv);

  // /* Runtime. */
  // WM_init(C, argc, (const char **)argv);

  // CTX_py_init_set(C, true);

  // WM_main(C);

  return KRAKEN_SUCCESS;
}
