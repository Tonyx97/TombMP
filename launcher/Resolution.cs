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
using System.Runtime.InteropServices;
using System.IO;
using System.Text.RegularExpressions;

[StructLayout(LayoutKind.Sequential)]
public struct POINTL
{
    [MarshalAs(UnmanagedType.I4)]
    public int x;
    [MarshalAs(UnmanagedType.I4)]
    public int y;
}

[StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
public struct DEVMODE
{
    // You can define the following constant  
    // but OUTSIDE the structure because you know  
    // that size and layout of the structure  
    // is very important  
    // CCHDEVICENAME = 32 = 0x50  
    [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
    public string dmDeviceName;
    // In addition you can define the last character array  
    // as following:  
    //[MarshalAs(UnmanagedType.ByValArray, SizeConst = 32)]  
    //public Char[] dmDeviceName;  
    // After the 32-bytes array  
    [MarshalAs(UnmanagedType.U2)]
    public UInt16 dmSpecVersion;
    [MarshalAs(UnmanagedType.U2)]
    public UInt16 dmDriverVersion;
    [MarshalAs(UnmanagedType.U2)]
    public UInt16 dmSize;
    [MarshalAs(UnmanagedType.U2)]
    public UInt16 dmDriverExtra;
    [MarshalAs(UnmanagedType.U4)]
    public UInt32 dmFields;
    public POINTL dmPosition;
    [MarshalAs(UnmanagedType.U4)]
    public UInt32 dmDisplayOrientation;
    [MarshalAs(UnmanagedType.U4)]
    public UInt32 dmDisplayFixedOutput;
    [MarshalAs(UnmanagedType.I2)]
    public Int16 dmColor;
    [MarshalAs(UnmanagedType.I2)]
    public Int16 dmDuplex;
    [MarshalAs(UnmanagedType.I2)]
    public Int16 dmYResolution;
    [MarshalAs(UnmanagedType.I2)]
    public Int16 dmTTOption;
    [MarshalAs(UnmanagedType.I2)]
    public Int16 dmCollate;
    // CCHDEVICENAME = 32 = 0x50  
    [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
    public string dmFormName;
    // Also can be defined as  
    //[MarshalAs(UnmanagedType.ByValArray,  
    // SizeConst = 32, ArraySubType = UnmanagedType.U1)]  
    //public Byte[] dmFormName;  
    [MarshalAs(UnmanagedType.U2)]
    public UInt16 dmLogPixels;
    [MarshalAs(UnmanagedType.U4)]
    public UInt32 dmBitsPerPel;
    [MarshalAs(UnmanagedType.U4)]
    public UInt32 dmPelsWidth;
    [MarshalAs(UnmanagedType.U4)]
    public UInt32 dmPelsHeight;
    [MarshalAs(UnmanagedType.U4)]
    public UInt32 dmDisplayFlags;
    [MarshalAs(UnmanagedType.U4)]
    public UInt32 dmDisplayFrequency;
    [MarshalAs(UnmanagedType.U4)]
    public UInt32 dmICMMethod;
    [MarshalAs(UnmanagedType.U4)]
    public UInt32 dmICMIntent;
    [MarshalAs(UnmanagedType.U4)]
    public UInt32 dmMediaType;
    [MarshalAs(UnmanagedType.U4)]
    public UInt32 dmDitherType;
    [MarshalAs(UnmanagedType.U4)]
    public UInt32 dmReserved1;
    [MarshalAs(UnmanagedType.U4)]
    public UInt32 dmReserved2;
    [MarshalAs(UnmanagedType.U4)]
    public UInt32 dmPanningWidth;
    [MarshalAs(UnmanagedType.U4)]
    public UInt32 dmPanningHeight;
}

namespace launcher
{
    class Resolution
    {
        [DllImport("User32.dll")]
        [return: MarshalAs(UnmanagedType.Bool)]
        public static extern Boolean EnumDisplaySettings([param: MarshalAs(UnmanagedType.LPTStr)] string lpszDeviceName, [param: MarshalAs(UnmanagedType.U4)] int iModeNum, [In, Out] ref DEVMODE lpDevMode);
    }
}