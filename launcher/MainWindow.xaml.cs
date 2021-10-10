using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Threading;
using System.ComponentModel;
using Microsoft.Win32;
using System.Runtime.InteropServices;
using System.IO;

namespace launcher
{
    public class ServerItem
    {
        public string ip { get; set; }
        public string name { get; set; }
        public string gamemode { get; set; }
        public string map { get; set; }
        public string players_count { get; set; }
        public string max_players { get; set; }
        public string players_text { get; set; }
        public List<string> players_list { get; set; }
    }

    public class KeyItem
    {
        public string name { get; set; }
        public int value { get; set; }

        public override string ToString()
        {
            return name;
        }
    }

    public class ResolutionItem
    {
        public uint x { get; set; }
        public uint y { get; set; }

        public override string ToString()
        {
            return x.ToString() + "x" + y.ToString();
        }
    }

    public class AspectItem
    {
        public uint x { get; set; }
        public uint y { get; set; }

        public override string ToString()
        {
            return x.ToString() + ":" + y.ToString();
        }
    }

    public partial class MainWindow : Window
    {
        private static string GAME_NAME = "TombMP.exe";

        private string game_path,
                       game_full_path,
                       game_hash,
                       sv_ip,
                       curr_control = null;

        private System.Windows.Threading.DispatcherTimer game_checker;

        private System.Diagnostics.Process game;

        private uint screen_x, screen_y;
        private uint aspect_x, aspect_y;
        private uint game_fov = 80;

        private bool updating_sv_list = false,
                     game_running = false,
                     wnd_mode = false,
                     resetting_fov = true;

        private Mutex mtx;

        public void display_error(string str, bool close = true)
        {
            System.Windows.MessageBox.Show(str, "TombMP Launcher", MessageBoxButton.OK, MessageBoxImage.Exclamation);

            if (close)
                System.Windows.Application.Current.Shutdown();
        }

        public string get_registry_data(string key, string name)
        {
            var current_user = Registry.CurrentUser;
            var main_key = current_user.CreateSubKey("SOFTWARE\\TombMP");
            var value_key = main_key.CreateSubKey(key);

            var value = value_key.GetValue(name);

            value_key.Close();
            main_key.Close();

            return (value == null ? "" : value.ToString());
        }

        public int get_registry_data_int(string key, string name)
        {
            var current_user = Registry.CurrentUser;
            var main_key = current_user.CreateSubKey("SOFTWARE\\TombMP");
            var value_key = main_key.CreateSubKey(key);

            var value = value_key.GetValue(name);

            value_key.Close();
            main_key.Close();

            return (value == null ? -1 : int.Parse(value.ToString()));
        }

        public void set_registry_data(string key, string name, string value)
        {
            var current_user = Registry.CurrentUser;
            var main_key = current_user.CreateSubKey("SOFTWARE\\TombMP");
            var value_key = main_key.CreateSubKey(key);

            value_key.SetValue(name, value);

            value_key.Close();
            main_key.Close();
        }

        public void set_registry_data(string key, string name, int value)
        {
            var current_user = Registry.CurrentUser;
            var main_key = current_user.CreateSubKey("SOFTWARE\\TombMP");
            var value_key = main_key.CreateSubKey(key);

            value_key.SetValue(name, value);

            value_key.Close();
            main_key.Close();
        }

        public void init_controls()
        {
            key_control.Items.Add("Forward");
            key_control.Items.Add("Back");
            key_control.Items.Add("Left");
            key_control.Items.Add("Right");
            key_control.Items.Add("Jump");
            key_control.Items.Add("Draw Guns");
            key_control.Items.Add("Action");
            key_control.Items.Add("Walk");
            key_control.Items.Add("Option");
            key_control.Items.Add("Look");
            key_control.Items.Add("Roll");
            key_control.Items.Add("Flare");
            key_control.Items.Add("Select");
            key_control.Items.Add("Deselect");
            key_control.Items.Add("Duck");
            key_control.Items.Add("Sprint");
			key_control.Items.Add("SmallMedKit");
			key_control.Items.Add("LargeMedKit");

			key_key.Items.Add(new KeyItem { name = "Space Bar", value = (int)System.Windows.Forms.Keys.Space });
			key_key.Items.Add(new KeyItem { name = "Arrow Up", value = (int)System.Windows.Forms.Keys.Up });
            key_key.Items.Add(new KeyItem { name = "Arrow Down", value = (int)System.Windows.Forms.Keys.Down });
            key_key.Items.Add(new KeyItem { name = "Arrow Left", value = (int)System.Windows.Forms.Keys.Left });
            key_key.Items.Add(new KeyItem { name = "Arrow Right", value = (int)System.Windows.Forms.Keys.Right });
            key_key.Items.Add(new KeyItem { name = "TAB", value = (int)System.Windows.Forms.Keys.Tab });
            key_key.Items.Add(new KeyItem { name = "Shift", value = (int)System.Windows.Forms.Keys.ShiftKey });
            key_key.Items.Add(new KeyItem { name = "Control", value = (int)System.Windows.Forms.Keys.ControlKey });
            key_key.Items.Add(new KeyItem { name = "Alt", value = (int)System.Windows.Forms.Keys.Menu });
            key_key.Items.Add(new KeyItem { name = "Escape", value = (int)System.Windows.Forms.Keys.Escape });
            key_key.Items.Add(new KeyItem { name = "Return", value = (int)System.Windows.Forms.Keys.Return });
            key_key.Items.Add(new KeyItem { name = "Enter", value = (int)System.Windows.Forms.Keys.Enter });
            key_key.Items.Add(new KeyItem { name = "F1", value = (int)System.Windows.Forms.Keys.F1 });
            key_key.Items.Add(new KeyItem { name = "F2", value = (int)System.Windows.Forms.Keys.F2 });
            key_key.Items.Add(new KeyItem { name = "F3", value = (int)System.Windows.Forms.Keys.F3 });
            key_key.Items.Add(new KeyItem { name = "F4", value = (int)System.Windows.Forms.Keys.F4 });
            key_key.Items.Add(new KeyItem { name = "F5", value = (int)System.Windows.Forms.Keys.F5 });
            key_key.Items.Add(new KeyItem { name = "F6", value = (int)System.Windows.Forms.Keys.F6 });
            key_key.Items.Add(new KeyItem { name = "F7", value = (int)System.Windows.Forms.Keys.F7 });
            key_key.Items.Add(new KeyItem { name = "F8", value = (int)System.Windows.Forms.Keys.F8 });
            key_key.Items.Add(new KeyItem { name = "F9", value = (int)System.Windows.Forms.Keys.F9 });
            key_key.Items.Add(new KeyItem { name = "F10", value = (int)System.Windows.Forms.Keys.F10 });
            key_key.Items.Add(new KeyItem { name = "F11", value = (int)System.Windows.Forms.Keys.F11 });
            key_key.Items.Add(new KeyItem { name = "F12", value = (int)System.Windows.Forms.Keys.F12 });
            key_key.Items.Add(new KeyItem { name = "Numpad 0", value = (int)System.Windows.Forms.Keys.NumPad0 });
            key_key.Items.Add(new KeyItem { name = "Numpad 1", value = (int)System.Windows.Forms.Keys.NumPad1 });
            key_key.Items.Add(new KeyItem { name = "Numpad 2", value = (int)System.Windows.Forms.Keys.NumPad2 });
            key_key.Items.Add(new KeyItem { name = "Numpad 3", value = (int)System.Windows.Forms.Keys.NumPad3 });
            key_key.Items.Add(new KeyItem { name = "Numpad 4", value = (int)System.Windows.Forms.Keys.NumPad4 });
            key_key.Items.Add(new KeyItem { name = "Numpad 5", value = (int)System.Windows.Forms.Keys.NumPad5 });
            key_key.Items.Add(new KeyItem { name = "Numpad 6", value = (int)System.Windows.Forms.Keys.NumPad6 });
            key_key.Items.Add(new KeyItem { name = "Numpad 7", value = (int)System.Windows.Forms.Keys.NumPad7 });
            key_key.Items.Add(new KeyItem { name = "Numpad 8", value = (int)System.Windows.Forms.Keys.NumPad8 });
            key_key.Items.Add(new KeyItem { name = "Numpad 9", value = (int)System.Windows.Forms.Keys.NumPad9 });
            key_key.Items.Add(new KeyItem { name = "Insert", value = (int)System.Windows.Forms.Keys.Insert });
            key_key.Items.Add(new KeyItem { name = "Delete", value = (int)System.Windows.Forms.Keys.Delete });
            key_key.Items.Add(new KeyItem { name = "Home", value = (int)System.Windows.Forms.Keys.Home });
            key_key.Items.Add(new KeyItem { name = "End", value = (int)System.Windows.Forms.Keys.End });
            key_key.Items.Add(new KeyItem { name = "Next Page", value = (int)System.Windows.Forms.Keys.PageUp });
            key_key.Items.Add(new KeyItem { name = "Prior Page", value = (int)System.Windows.Forms.Keys.PageDown });
            key_key.Items.Add(new KeyItem { name = "A", value = (int)System.Windows.Forms.Keys.A });
            key_key.Items.Add(new KeyItem { name = "B", value = (int)System.Windows.Forms.Keys.B });
            key_key.Items.Add(new KeyItem { name = "C", value = (int)System.Windows.Forms.Keys.C });
            key_key.Items.Add(new KeyItem { name = "D", value = (int)System.Windows.Forms.Keys.D });
            key_key.Items.Add(new KeyItem { name = "E", value = (int)System.Windows.Forms.Keys.E });
            key_key.Items.Add(new KeyItem { name = "F", value = (int)System.Windows.Forms.Keys.F });
            key_key.Items.Add(new KeyItem { name = "G", value = (int)System.Windows.Forms.Keys.G });
            key_key.Items.Add(new KeyItem { name = "H", value = (int)System.Windows.Forms.Keys.H });
            key_key.Items.Add(new KeyItem { name = "I", value = (int)System.Windows.Forms.Keys.I });
            key_key.Items.Add(new KeyItem { name = "J", value = (int)System.Windows.Forms.Keys.J });
            key_key.Items.Add(new KeyItem { name = "K", value = (int)System.Windows.Forms.Keys.K });
            key_key.Items.Add(new KeyItem { name = "L", value = (int)System.Windows.Forms.Keys.L });
            key_key.Items.Add(new KeyItem { name = "M", value = (int)System.Windows.Forms.Keys.M });
            key_key.Items.Add(new KeyItem { name = "N", value = (int)System.Windows.Forms.Keys.N });
            key_key.Items.Add(new KeyItem { name = "O", value = (int)System.Windows.Forms.Keys.O });
            key_key.Items.Add(new KeyItem { name = "P", value = (int)System.Windows.Forms.Keys.P });
            key_key.Items.Add(new KeyItem { name = "Q", value = (int)System.Windows.Forms.Keys.Q });
            key_key.Items.Add(new KeyItem { name = "R", value = (int)System.Windows.Forms.Keys.R });
            key_key.Items.Add(new KeyItem { name = "S", value = (int)System.Windows.Forms.Keys.S });
            key_key.Items.Add(new KeyItem { name = "T", value = (int)System.Windows.Forms.Keys.T });
            key_key.Items.Add(new KeyItem { name = "U", value = (int)System.Windows.Forms.Keys.U });
            key_key.Items.Add(new KeyItem { name = "V", value = (int)System.Windows.Forms.Keys.V });
            key_key.Items.Add(new KeyItem { name = "W", value = (int)System.Windows.Forms.Keys.W });
            key_key.Items.Add(new KeyItem { name = "X", value = (int)System.Windows.Forms.Keys.X });
            key_key.Items.Add(new KeyItem { name = "Y", value = (int)System.Windows.Forms.Keys.Y });
            key_key.Items.Add(new KeyItem { name = "Z", value = (int)System.Windows.Forms.Keys.Z });
			key_key.Items.Add(new KeyItem { name = "Z", value = (int)System.Windows.Forms.Keys.Z });
			key_key.Items.Add(new KeyItem { name = "0", value = (int)System.Windows.Forms.Keys.D0 });
			key_key.Items.Add(new KeyItem { name = "1", value = (int)System.Windows.Forms.Keys.D1 });
			key_key.Items.Add(new KeyItem { name = "2", value = (int)System.Windows.Forms.Keys.D2 });
			key_key.Items.Add(new KeyItem { name = "3", value = (int)System.Windows.Forms.Keys.D3 });
			key_key.Items.Add(new KeyItem { name = "4", value = (int)System.Windows.Forms.Keys.D4 });
			key_key.Items.Add(new KeyItem { name = "5", value = (int)System.Windows.Forms.Keys.D5 });
			key_key.Items.Add(new KeyItem { name = "6", value = (int)System.Windows.Forms.Keys.D6 });
			key_key.Items.Add(new KeyItem { name = "7", value = (int)System.Windows.Forms.Keys.D7 });
			key_key.Items.Add(new KeyItem { name = "8", value = (int)System.Windows.Forms.Keys.D8 });
			key_key.Items.Add(new KeyItem { name = "9", value = (int)System.Windows.Forms.Keys.D9 });

			if (get_registry_data_int("game", "forward") == -1)
            {
                set_registry_data("game", "Forward", (int)System.Windows.Forms.Keys.Up);
                set_registry_data("game", "Back", (int)System.Windows.Forms.Keys.Down);
                set_registry_data("game", "Left", (int)System.Windows.Forms.Keys.Left);
                set_registry_data("game", "Right", (int)System.Windows.Forms.Keys.Right);
                set_registry_data("game", "Jump", (int)System.Windows.Forms.Keys.Menu);
                set_registry_data("game", "Draw Guns", (int)System.Windows.Forms.Keys.Space);
                set_registry_data("game", "Action", (int)System.Windows.Forms.Keys.ControlKey);
                set_registry_data("game", "Walk", (int)System.Windows.Forms.Keys.ShiftKey);
                set_registry_data("game", "Option", (int)System.Windows.Forms.Keys.Escape);
                set_registry_data("game", "Look", (int)System.Windows.Forms.Keys.NumPad0);
                set_registry_data("game", "Roll", (int)System.Windows.Forms.Keys.End);
                set_registry_data("game", "Flare", (int)System.Windows.Forms.Keys.F);
                set_registry_data("game", "Select", (int)System.Windows.Forms.Keys.Enter);
                set_registry_data("game", "Deselect", (int)System.Windows.Forms.Keys.Escape);
                set_registry_data("game", "Duck", (int)System.Windows.Forms.Keys.Q);
                set_registry_data("game", "Sprint", (int)System.Windows.Forms.Keys.E);
				set_registry_data("game", "SmallMedKit", (int)System.Windows.Forms.Keys.D9);
				set_registry_data("game", "LargeMedKit", (int)System.Windows.Forms.Keys.D0);
			}
        }

        public void init_resolutions()
        {
            DEVMODE mode = new DEVMODE();

            mode.dmSize = (ushort)Marshal.SizeOf(mode);

            int modeIndex = 0;

            List<KeyValuePair<uint, uint>> resolutions = new List<KeyValuePair<uint, uint>>();

            int bpp = -1;

            while (Resolution.EnumDisplaySettings(null, modeIndex++, ref mode))
            {
                if (bpp == -1)
                    bpp = (int)mode.dmBitsPerPel;

                try
                {
                    var pair = new KeyValuePair<uint, uint>(mode.dmPelsWidth, mode.dmPelsHeight);

                    if (mode.dmBitsPerPel == bpp)
                    {
                        if (!resolutions.Contains(pair))
                            resolutions.Add(pair);
                    }
                    else break;
                }
                catch (Exception e) { e.Message.ToString(); }
            }

            foreach (var res in resolutions.OrderBy(i => i.Key))
                resolution.Items.Add(new ResolutionItem { x = res.Key, y = res.Value });

            aspect.Items.Add(new AspectItem { x = 1, y = 1 });
            aspect.Items.Add(new AspectItem { x = 4, y = 1 });
            aspect.Items.Add(new AspectItem { x = 4, y = 3 });
            aspect.Items.Add(new AspectItem { x = 3, y = 2 });
            aspect.Items.Add(new AspectItem { x = 5, y = 4 });
            aspect.Items.Add(new AspectItem { x = 16, y = 9 });
            aspect.Items.Add(new AspectItem { x = 16, y = 10 });
            aspect.Items.Add(new AspectItem { x = 17, y = 9 });
            aspect.Items.Add(new AspectItem { x = 21, y = 9 });
            aspect.Items.Add(new AspectItem { x = 32, y = 9 });
        }
        private int get_resolution_index(uint x, uint y)
        {
            int i = 0;

            foreach (ResolutionItem res in resolution.Items)
            {
                if (res.x == x && res.y == y)
                    return i;

                ++i;
            }

            return -1;
        }
        private int get_aspect_index(uint x, uint y)
        {
            int i = 0;

            foreach (AspectItem asp in aspect.Items)
            {
                if (asp.x == x && asp.y == y)
                    return i;

                ++i;
            }

            return -1;
        }

        public List<ServerItem> refresh_server_list()
        {
            if (!Net.send(packet_id.PID_SERVERS_LIST))
                return null;

            packet_id pid;

            int size = 0;

            var servers_list = Net.rcv(out pid, out size);
            if (servers_list == null)
                return null;

            var list_str = Encoding.ASCII.GetString(servers_list).Trim(new char[] { '\0' });

            if (!list_str.Equals("__LIST_EMPTY__"))
            {
                var servers = list_str.Split(new string[] { ":" }, StringSplitOptions.RemoveEmptyEntries);

                List<ServerItem> list = new List<ServerItem>();

                foreach (var sv in servers)
                {
                    if (sv.Equals("__EMPTY__"))
                        continue;

                    var sv_info = sv.Split(new string[] { ";" }, StringSplitOptions.RemoveEmptyEntries);

                    var item = new ServerItem
                    {
                        ip = sv_info[0],
                        name = sv_info[1],
                        gamemode = sv_info[2],
                        players_count = sv_info[3],
                        max_players = sv_info[4],
                        players_text = sv_info[3] + " / " + sv_info[4],
                        map = sv_info[5]
                    };

                    item.players_list = new List<string>();

                    for (int i = 6; i < sv_info.Length; ++i)
                        item.players_list.Add(sv_info[i]);

                    list.Add(item);
                }

                return list;
            }

            return null;
        }

        private void check_game(object sender, EventArgs e)
        {
            if (game == null)
                return;

            if (game.HasExited)
            {
                if (game.ExitCode != 0)
                {
                    switch (game.ExitCode)
                    {
                        case 1: display_error("DirectX could not be initialized", false); break;
                        case 2: display_error("Could not initialize basic systems", false); break;
                        case 3: display_error("Could not establish connection", false); break;
                        case 4: display_error("Could not connect to the server", false); break;
                        case 5: display_error("Could not initialize client", false); break;
                        case 6: display_error("Invalid parameters", false); break;
                        case 7: display_error("Could not initialize bug ripper", false); break;
                        case 8: display_error("Could not initialize UI system", false); break;
                        case 21: display_error("Connection lost", false); break;
                        case 22: display_error("Invalid password", false); break;
                        case 23: display_error("You are banned from this server", false); break;
                        case 24: display_error("Connection attempt failed", false); break;
                        default: display_error("Unknown error: " + game.ExitCode.ToString(), false); break;
                    }
                }

                game_running = false;
                game = null;

                connect_btn.Content = "Connect";
                connect_btn.IsEnabled = true;
            }
        }

        public void OnWindowClosing(object sender, CancelEventArgs e)
        {
            if (game != null)
                game.CloseMainWindow();

            Net.destroy();

            mtx.Close();
        }

        public MainWindow()
        {
            bool created = true;

            mtx = new Mutex(true, "tombmp", out created);

            if (!created)
            {
                display_error("Only one instance of the launcher allowed.");
                return;
            }

#if DEBUG
            if (!Net.connect("127.0.0.1"))
#else
			if (!Net.connect("217.182.174.42"))
#endif
            {
                display_error("Could not connect to the MasterServer.");
                return;
            }

            if (!Net.send(packet_id.PID_MS_CLIENT_TYPE, Net.TYPE()) ||
                !Net.send(packet_id.PID_CLIENT_VER, Net.VERSION()))
            {
                display_error("Failed sending basic info.");
                return;
            }

            packet_id pid;

            int size = 0;

            var game_hash_arr = Net.rcv(out pid, out size);

            if (pid != packet_id.PID_OK)
            {
                display_error("The launcher is outdated, please download the latest version and try again.");
                return;
            }

            game_hash = Encoding.ASCII.GetString(game_hash_arr).Trim(new char[] { '\0' });

            InitializeComponent();

            Closing += OnWindowClosing;

            init_resolutions();
            init_controls();

            game_checker = new System.Windows.Threading.DispatcherTimer();
            game_checker.Tick += new EventHandler(check_game);
            game_checker.Interval = new TimeSpan(0, 0, 1);
            game_checker.Start();

            game_path = Environment.CurrentDirectory + "\\patch\\";
            game_full_path = game_path + GAME_NAME;

            nickname.Text = get_registry_data("net", "nickname");
            pass.Text = get_registry_data("net", "last_password");
            last_sv.Content = get_registry_data("net", "last_sv_name");
            sv_ip = get_registry_data("net", "last_sv_ip");
            wnd_mode = get_registry_data("game", "window_mode").Equals("fullscreen");

            uint temp_game_fov;

            uint.TryParse(get_registry_data("game", "res_x"), out screen_x);
            uint.TryParse(get_registry_data("game", "res_y"), out screen_y);
            uint.TryParse(get_registry_data("game", "aspect_x"), out aspect_x);
            uint.TryParse(get_registry_data("game", "aspect_y"), out aspect_y);
            uint.TryParse(get_registry_data("game", "fov"), out temp_game_fov);

            fov.Value = (temp_game_fov >= 1 && temp_game_fov <= 180 ? temp_game_fov : 80);

            if (screen_x > 0 && screen_x < 10000 && screen_y > 0 && screen_y < 10000)
            {
                int res_index = get_resolution_index(screen_x, screen_y);

                if (res_index != -1)
                    resolution.SelectedIndex = res_index;
            }

            if (aspect_x > 0 && aspect_x < 50 && aspect_y > 0 && aspect_y < 50)
            {
                int asp_index = get_aspect_index(aspect_x, aspect_y);

                if (asp_index != -1)
                    aspect.SelectedIndex = asp_index;
            }

            fullscreen.IsChecked = wnd_mode;

            if (String.IsNullOrEmpty(last_sv.Content.ToString()))
                last_sv.Content = "None";

            players.Visibility = Visibility.Hidden;

            updating_sv_list = true;

            var new_list = refresh_server_list();

            updating_sv_list = false;

            refresh.Visibility = Visibility.Visible;
            refresh_spin.Visibility = Visibility.Hidden;

            if (new_list == null)
                return;

            foreach (var sv in new_list)
                sv_list.Items.Add(sv);
        }

        private async void refresh_Click(object sender, RoutedEventArgs e)
        {
            if (updating_sv_list)
                return;

            updating_sv_list = true;

            refresh.Visibility = Visibility.Hidden;
            refresh_spin.Visibility = Visibility.Visible;

            players.Visibility = Visibility.Hidden;

            sv_list.Items.Clear();

            var new_list = await Task.Run(refresh_server_list);

            refresh.Visibility = Visibility.Visible;
            refresh_spin.Visibility = Visibility.Hidden;

            updating_sv_list = false;

            if (new_list == null)
                return;

            foreach (var sv in new_list)
                sv_list.Items.Add(sv);
        }

        private void sv_list_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            var lv = sender as System.Windows.Controls.ListView;

            if (lv.SelectedIndex == -1)
                return;

            var item = lv.SelectedItem as ServerItem;

            players.Items.Clear();

            foreach (var player in item.players_list)
                players.Items.Add(player);

            sv_ip = item.ip;

            players.Visibility = Visibility.Visible;
        }

        private void fullscreen_Click(object sender, RoutedEventArgs e)
        {
            var checkbox = sender as System.Windows.Controls.CheckBox;

            set_registry_data("game", "window_mode", (wnd_mode = (checkbox.IsChecked == true)) ? "fullscreen" : "windowed");
        }

        private void key_control_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            var res = sender as System.Windows.Controls.ComboBox;

            if (res.SelectedItem != null)
                curr_control = res.SelectedItem.ToString();
            else curr_control = null;
        }

        private void key_key_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            var res = sender as System.Windows.Controls.ComboBox;

            if (curr_control != null)
            {
                set_registry_data("game", curr_control, (res.SelectedItem as KeyItem).value);
            }
        }

        private void resolution_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            var res = sender as System.Windows.Controls.ComboBox;

            if (res.SelectedItem != null)
            {
                set_registry_data("game", "res_x", (res.SelectedItem as ResolutionItem).x.ToString());
                set_registry_data("game", "res_y", (res.SelectedItem as ResolutionItem).y.ToString());
            }
        }
        private void aspect_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            var asp = sender as System.Windows.Controls.ComboBox;

            if (asp.SelectedItem != null)
            {
                set_registry_data("game", "aspect_x", (aspect_x = (asp.SelectedItem as AspectItem).x).ToString());
                set_registry_data("game", "aspect_y", (aspect_y = (asp.SelectedItem as AspectItem).y).ToString());
            }
        }
        private void fov_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            if (resetting_fov)
            {
                resetting_fov = false;
                return;
            }

            var fov_slider = sender as System.Windows.Controls.Slider;

            set_registry_data("game", "fov", (game_fov = (uint)fov_slider.Value).ToString());

            fov_lbl.Content = "FOV (" + game_fov + "):";
        }
        private void players_PreviewMouseDown(object sender, MouseButtonEventArgs e)
        {
            e.Handled = true;
        }

        private void nickname_TextChanged(object sender, TextChangedEventArgs e)
        {
            if (nickname.Text.Length > 2)
                set_registry_data("net", "nickname", nickname.Text);
        }

        private void pass_TextChanged(object sender, TextChangedEventArgs e)
        {
            if (pass.Text.Length > 0)
                set_registry_data("net", "last_password", pass.Text);
        }

        private void apply_settings(string path)
        {
            string[] file_data = File.ReadAllLines(path + "dgVoodoo.conf");

            int i = 0;

            bool checking_resolution = false;

            foreach (var line in file_data)
            {
                if (line.IndexOf("GameRealFSMode") == 1)
                    file_data[i] = ";GameRealFSMode = " + wnd_mode.ToString().ToLower();
                else if (line.IndexOf("Aspect") == 1)
                    file_data[i] = ";Aspect = " + aspect_x.ToString() + ":" + aspect_y.ToString();
                else if (line.IndexOf("FOV") == 1)
                    file_data[i] = ";FOV = " + game_fov.ToString();
                else if (line.Contains("internal3D"))
                    checking_resolution = true;
                else if (checking_resolution)
                {
                    var res_item = resolution.SelectedItem as ResolutionItem;

                    if (res_item != null)
                    {
                        if (line.IndexOf("Resolution") == 0 && res_item.x != 0 && res_item.y != 0)
                            file_data[i] = "Resolution = h:" + res_item.x.ToString() + ", v:" + res_item.y.ToString();
                    }
                }

                ++i;
            }

            File.WriteAllLines(path + "dgVoodoo.conf", file_data);
        }

        private void connect_btn_Click(object sender, RoutedEventArgs e)
        {
            if (sv_list.SelectedIndex == -1 && String.IsNullOrEmpty(sv_ip))
            {
                display_error("Select a server", false);
                return;
            }

            if (nickname.Text.Length <= 2)
            {
                display_error("Please enter a valid nickname", false);
                return;
            }

            if (sv_ip.Length < 8)
            {
                display_error("Invalid Server IP", false);
                return;
            }

            var hash = Utils.hash_file(game_full_path);

            if (hash == null)
            {
                display_error("The path to the game is invalid.", false);
                return;
            }

#if !DEBUG
			if (!hash.Equals(game_hash))
			{
				display_error("The game is outdated, please download the latest version.", false);
				return;
			}
#endif

            apply_settings(game_path);

            var info = new System.Diagnostics.ProcessStartInfo();

            info.FileName = game_full_path;
            info.WorkingDirectory = game_path;
            info.Arguments = "-ip " + sv_ip + " ";
            info.Arguments += "-pass " + (String.IsNullOrEmpty(pass.Text) ? "NO_PASS" : pass.Text) + " ";
            info.Arguments += "-nickname " + nickname.Text + " ";

            connect_btn.Content = "Launching...";
            connect_btn.IsEnabled = false;

            try { game = System.Diagnostics.Process.Start(info); }
            catch { display_error("Could not launch the game", false); }

            game_running = (game != null);

            if (game_running)
            {
                connect_btn.Content = "Running...";
                connect_btn.IsEnabled = false;

                if (sv_list.SelectedItem != null)
                    last_sv.Content = ((sv_list.SelectedItem as ServerItem).name);

                set_registry_data("net", "last_sv_ip", sv_ip);
                set_registry_data("net", "last_sv_name", last_sv.Content.ToString());
            }
            else
            {
                connect_btn.Content = "Connect";
                connect_btn.IsEnabled = true;
            }
        }
    }
}
