using System;
using System.Drawing;
using System.IO.Ports;
using System.Linq;
using System.Text.RegularExpressions;
using System.Windows.Forms;

ApplicationConfiguration.Initialize(); Application.Run(new MainForm());
sealed class MainForm : Form {
  readonly ComboBox ports=new(){DropDownStyle=ComboBoxStyle.DropDownList}; readonly Label state=new(){AutoSize=true}; readonly ProgressBar gas=new(){Minimum=0,Maximum=100,Height=38,Dock=DockStyle.Top}; readonly Label values=new(){AutoSize=true}; readonly Label buttons=new(){AutoSize=true}; readonly ComboBox debounce=new(){DropDownStyle=ComboBoxStyle.DropDownList}; SerialPort? serial; readonly System.Windows.Forms.Timer timer=new(){Interval=250};
  public MainForm(){Text="SETUP GAS – Configurazione acceleratore";Width=570;Height=390;var panel=new FlowLayoutPanel{Dock=DockStyle.Fill,FlowDirection=FlowDirection.TopDown,Padding=new Padding(18),AutoScroll=true}; Controls.Add(panel);
    panel.Controls.Add(new Label{Text="Porta COM",AutoSize=true});panel.Controls.Add(ports); var refresh=new Button{Text="Aggiorna porte"};refresh.Click+=(s,e)=>Find();panel.Controls.Add(refresh);var connect=new Button{Text="Connetti"};connect.Click+=(s,e)=>Connect();panel.Controls.Add(connect);panel.Controls.Add(state);panel.Controls.Add(gas);panel.Controls.Add(values);panel.Controls.Add(buttons);
    foreach(var x in new[]{"Rapida (3 ms)","Standard (5 ms)","Più stabile (12 ms)","Personalizzata"})debounce.Items.Add(x);debounce.SelectedIndex=1;panel.Controls.Add(new Label{Text="Cambio",AutoSize=true});panel.Controls.Add(debounce);var custom=new NumericUpDown{Minimum=2,Maximum=30,Value=5};panel.Controls.Add(custom);
    var test=new Button{Text="Applica per provare"};test.Click+=(s,e)=>Send("DEBOUNCE "+Debounce(custom));panel.Controls.Add(test);var save=new Button{Text="Salva filtro sull'ESP32"};save.Click+=(s,e)=>{Send("DEBOUNCE "+Debounce(custom));Send("DEBOUNCE SAVE");};panel.Controls.Add(save);
    foreach(var p in new[]{("Imposta 0%", "CAL ZERO"),("Imposta 100%", "CAL FULL"),("Salva sull'ESP32", "CAL SAVE"),("Azzera contatori", "COUNTERS CLEAR")}){var b=new Button{Text=p.Item1,AutoSize=true,Font=new Font(Font.FontFamily,12)};b.Click+=(s,e)=>Send(p.Item2);panel.Controls.Add(b);} timer.Tick+=(s,e)=>Poll(); Find(); }
  int Debounce(NumericUpDown n)=>debounce.SelectedIndex switch{0=>3,1=>5,2=>12,_=>(int)n.Value};
  void Find(){ports.Items.Clear();ports.Items.AddRange(SerialPort.GetPortNames());if(ports.Items.Count>0)ports.SelectedIndex=0;}
  void Connect(){try{timer.Stop();serial?.Dispose();serial=new SerialPort(ports.Text,115200){NewLine="\n",ReadTimeout=300};serial.Open();Send("ID?");state.Text="Connesso: verifica identità…";timer.Start();}catch(Exception e){state.Text="Errore: "+e.Message;}}
  void Send(string s){try{if(serial?.IsOpen==true)serial.WriteLine(s);}catch{}}
  void Poll(){try{if(serial?.IsOpen!=true)return;serial.WriteLine("GET");Read();}catch{}}
  void Read(){try{var line=serial?.ReadExisting()??"";if(line.Contains("SETUP-GAS"))state.Text="Connesso a SETUP GAS, firmware "+line.Trim().Split(',').Last();var m=Regex.Match(line,@"ADC=(\d+) GAS=(\d+).*DEBOUNCE=(\d+).*UP=(\d+) DOWN=(\d+)");if(m.Success){gas.Value=Math.Clamp((int)(uint.Parse(m.Groups[2].Value)*100/65535),0,100);values.Text="ADC grezzo: "+m.Groups[1].Value+"   Gas: "+gas.Value+"%   Filtro attivo/salvato: "+m.Groups[3].Value+" ms";buttons.Text="Marcia su: "+m.Groups[4].Value+" conferme   Marcia giù: "+m.Groups[5].Value+" conferme";}}catch{}}
}
