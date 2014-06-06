
        /* 
    Copyright (c) 2012 Microsoft Corporation.  All rights reserved.
    Use of this sample source code is subject to the terms of the Microsoft license 
    agreement under which you licensed this sample source code and is provided AS-IS.
    If you did not accept the terms of the license agreement, you are not authorized 
    to use this sample source code.  For the terms of the license, please see the 
    license agreement between you and Microsoft.
  
    To see all Code Samples for Windows Phone, visit http://go.microsoft.com/fwlink/?LinkID=219604 
  
*/
using System;
using System.Collections.ObjectModel;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Input;
using Microsoft.Phone.Controls;
using Microsoft.Phone.Shell;
using Microsoft.Phone.Tasks;
using sdkBluetoothA2AWP8CS.Resources;
using Windows.Networking.Proximity;
using Windows.Networking.Sockets;
using Windows.Storage.Streams;
using kuntakinte;
using System.Windows.Threading;
using Microsoft.Devices.Sensors;

namespace sdkBluetoothA2AWP8CS
{
    public partial class MainPage : PhoneApplicationPage
    {
        ObservableCollection<PeerAppInfo> _peerApps;    // A local copy of peer app information
        StreamSocket _socket;                           // The socket object used to communicate with a peer
        string _peerName = string.Empty;                // The name of the current peer

     
        // Error code constants
        const uint ERR_BLUETOOTH_OFF = 0x8007048F;      // The Bluetooth radio is off
        const uint ERR_MISSING_CAPS = 0x80070005;       // A capability is missing from your WMAppManifest.xml
        const uint ERR_NOT_ADVERTISING = 0x8000000E;    // You are currently not advertising your presence using PeerFinder.Start()

       flightbox mflightbox;
        btConManager mConManager;


       

        float roll;
        float pitch;
        float yaw;

        float[] motors;

        //timer
        DispatcherTimer timer;

        Motion motion;

        //throttle
        float mthrottle;

        // Constructor
        public MainPage()
        {
            InitializeComponent();

            //SystemTray.SetProgressIndicator(this, new ProgressIndicator());

            this.DataContext = "Receivers";
            //init();
            motors = new float[4];

            //new bluetooth manager
            mConManager = new btConManager();


            mflightbox = new flightbox(); // initialize a new flightbox


            mflightbox.inclineEvent += fb_inclineEvent;

            //mflightbox.motorEvent += mflightbox_motorEvent;
            motion = new Motion();
            motion.TimeBetweenUpdates = TimeSpan.FromMilliseconds(5);
            motion.CurrentValueChanged += new EventHandler<SensorReadingEventArgs<MotionReading>>(motion_CurrentValueChanged);


            motion.Start();

            mConManager.Initialize();



            //timer = new DispatcherTimer();
            //timer.Interval = TimeSpan.FromSeconds(2);
            //timer.Tick += new EventHandler(timer_Tick);
            //timer.Start();
            mthrottle = 0;

        }

     

        protected override void OnNavigatedTo(System.Windows.Navigation.NavigationEventArgs e)
        {
            // Maintain a list of peers and bind that list to the UI
            _peerApps = new ObservableCollection<PeerAppInfo>();
            PeerList.ItemsSource = _peerApps;

            // Register for incoming connection requests
            PeerFinder.ConnectionRequested += PeerFinder_ConnectionRequested;

            // Start advertising ourselves so that our peers can find us
            PeerFinder.DisplayName = "Receivers";// App.ChatName;
            PeerFinder.Start();

            RefreshPeerAppList();

            base.OnNavigatedTo(e);
        }

        protected override void OnNavigatingFrom(System.Windows.Navigation.NavigatingCancelEventArgs e)
        {
            
            PeerFinder.ConnectionRequested -= PeerFinder_ConnectionRequested;

            
            base.OnNavigatingFrom(e);
        }

        void PeerFinder_ConnectionRequested(object sender, ConnectionRequestedEventArgs args)
        {
            try
            {
                this.Dispatcher.BeginInvoke(() =>
                {
                    // Ask the user if they want to accept the incoming request.
                    var result = MessageBox.Show(String.Format(AppResources.Msg_ChatPrompt, args.PeerInformation.DisplayName)
                                                 , AppResources.Msg_ChatPromptTitle, MessageBoxButton.OKCancel);
                    if (result == MessageBoxResult.OK)
                    {
                        ConnectToPeer(args.PeerInformation);
                    }
                    else
                    {
                        // Currently no method to tell the sender that the connection was rejected.
                    }
                });
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);

            }
        }

        async void ConnectToPeer(PeerInformation peer)
        {
            try
            {
                _socket = await PeerFinder.ConnectAsync(peer);

                // We can preserve battery by not advertising our presence.
                PeerFinder.Stop();

                _peerName = peer.DisplayName;
                //UpdateChatBox(AppResources.Msg_ChatStarted, true);

                // Since this is a chat, messages can be incoming and outgoing. 
                // Listen for incoming messages.
                ListenForIncomingMessage();
            }
            catch (Exception ex)
            {
                // In this sample, we handle each exception by displaying it and
                // closing any outstanding connection. An exception can occur here if, for example, 
                // the connection was refused, the connection timeout etc.
                MessageBox.Show(ex.Message);

            }
        }

        private DataReader _dataReader;
        private async void ListenForIncomingMessage()
        {
            try
            {
                int message = await GetMessage();

                // Add to chat
                //UpdateChatBox(message, true);
                mflightbox.throttle(message);
                throttleText.Text = "Throttle: "+message;
                // Start listening for the next message.
                ListenForIncomingMessage();
            }
            catch (Exception)
            {
                //UpdateChatBox(AppResources.Msg_ChatEnded, true);

            }
        }



        private async Task<int> GetMessage()
        {
            if (_dataReader == null)
                _dataReader = new DataReader(_socket.InputStream);

            // Each message is sent in two blocks.
            // The first is the size of the message.
            // The second if the message itself.
            //var len = await GetMessageSize();
            await _dataReader.LoadAsync(4);
            

            return _dataReader.ReadInt32();
            //uint messageLen = (uint)_dataReader.ReadInt32();
           // await _dataReader.LoadAsync(messageLen);
            //return _dataReader.ReadString(messageLen);
            
        }

        private void FindPeers_Tap(object sender, GestureEventArgs e)
        {
            RefreshPeerAppList();
        }

        /// <summary>
        /// Asynchronous call to re-populate the ListBox of peers.
        /// </summary>
        private async void RefreshPeerAppList()
        {
            try
            {
                //StartProgress("finding peers ...");
                var peers = await PeerFinder.FindAllPeersAsync();

                // By clearing the backing data, we are effectively clearing the ListBox
                _peerApps.Clear();

                if (peers.Count == 0)
                {
                    tbPeerList.Text = AppResources.Msg_NoPeers;
                }
                else
                {
                    tbPeerList.Text = String.Format(AppResources.Msg_FoundPeers, peers.Count);
                    // Add peers to list
                    foreach (var peer in peers)
                    {
                        _peerApps.Add(new PeerAppInfo(peer));
                    }

                    // If there is only one peer, go ahead and select it
                    if (PeerList.Items.Count == 1)
                        PeerList.SelectedIndex = 0;

                }
            }
            catch (Exception ex)
            {

                MessageBox.Show(ex.Message);
            }
        }
        

        private void ConnectToSelected_Tap_1(object sender, GestureEventArgs e)
        {
            if (PeerList.SelectedItem == null)
            {
                MessageBox.Show(AppResources.Err_NoPeer,AppResources.Err_NoConnectTitle, MessageBoxButton.OK);
                return;
            }

            // Connect to the selected peer.
            PeerAppInfo pdi = PeerList.SelectedItem as PeerAppInfo;
            PeerInformation peer = pdi.PeerInfo;

            ConnectToPeer(peer);
        }

        


        private void ShowBluetoothControlPanel()
        {
            ConnectionSettingsTask connectionSettingsTask = new ConnectionSettingsTask();
            connectionSettingsTask.ConnectionSettingsType = ConnectionSettingsType.Bluetooth;
            connectionSettingsTask.Show();
        }

            void motion_CurrentValueChanged(object sender, SensorReadingEventArgs<MotionReading> e)
        {

            float[] attitude = new float[3];
            attitude[0]=e.SensorReading.Attitude.Roll;
            attitude[1]=e.SensorReading.Attitude.Pitch;
            
            motors=mflightbox.compensate(attitude);

            mConManager.SendCommand(motors);
            Dispatcher.BeginInvoke(() =>
            {

                rollTextBlock.Text = attitude[0].ToString("f5");
                pitchTextBlock.Text = attitude[1].ToString("f5");
                motor0.Text = motors[0].ToString("f5");
                motor1.Text = motors[1].ToString("f5");
                motor2.Text = motors[2].ToString("f5");
                motor3.Text = motors[3].ToString("f5");

                //updateMotorDrive(data);
            });
            

        }

        void mflightbox_motorEvent(float[] data)
        {
            //throw new NotImplementedException();
            //updateMotorDrive(data);
            
           
            Dispatcher.BeginInvoke(() =>
            {

                
                motor0.Text = data[0].ToString();
                motor1.Text = data[1].ToString();
                motor2.Text = data[2].ToString();
                motor3.Text = data[3].ToString();

                //updateMotorDrive(data);
            });
            

            
           
        }

        void mflightbox_accelEvent(float[] __param0)
        {
            //updateMotorDrive(data);
            Dispatcher.BeginInvoke(() =>
            {
               
            });
        }


     
        private async void AppToDevice()
        {

            ConnectAppToDeviceButton.Content = "Connecting...";
            PeerFinder.AlternateIdentities["Bluetooth:Paired"] = "";
            var pairedDevices = await PeerFinder.FindAllPeersAsync();

            if (pairedDevices.Count == 0)
            {
                MessageBox.Show("No paired devices were found.");
            }
            else
            {
                foreach (var pairedDevice in pairedDevices)
                {
                    if (pairedDevice.DisplayName == DeviceName.Text)
                    {
                        mConManager.Connect(pairedDevice.HostName);
                        ConnectAppToDeviceButton.Content = "Connected";
                        DeviceName.IsReadOnly = true;
                        ConnectAppToDeviceButton.IsEnabled = false;
                        continue;
                    }
                }
            }
        }


        void fb_accelEvent(float[] data)
        {
            //updateMotorDrive(data);
            Dispatcher.BeginInvoke(() =>
            {
                
            });

        }

        void fb_inclineEvent(float[] data)
        {

            Dispatcher.BeginInvoke(() =>
            {
                roll = data[0];
                pitch = data[1];
                yaw = data[2];
                rollTextBlock.Text = roll.ToString("f5");
                pitchTextBlock.Text = pitch.ToString("f5");
                yawTextBlock.Text = yaw.ToString("f5");

                

            });

            //updateMotorDrive(roll);
            //updateMotorDrive(yaw);
            
        }


        private async void updateMotorDrive(int[] cmd)
        {
            //await mConManager.SendCommand(cmd);
        }

        private void ConnectAppToDeviceButton_Click(object sender, RoutedEventArgs e)
        {
            AppToDevice();
        }

        private async void Reco1_Click(object sender, RoutedEventArgs e)
        {
            //start recognition
            //SpeechRecognitionUIResult recoResult = await recoWithUI.RecognizeWithUIAsync();
        }

        private void Button_Click(object sender, RoutedEventArgs e)
        {
            //Reco1_Click(sender, e);
            //boutThatAction();
            timer.Start();
            //Listen();
        }

        private void throttleSlider_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
           mflightbox.throttle((float)e.NewValue);
        }




    }

    /// <summary>
    ///  Class to hold all peer information
    /// </summary>
    public class PeerAppInfo
    {
        internal PeerAppInfo(PeerInformation peerInformation)
        {
            this.PeerInfo = peerInformation;
            this.DisplayName = this.PeerInfo.DisplayName;
        }

        public string DisplayName { get; private set; }
        public PeerInformation PeerInfo { get; private set; }
    }

}
