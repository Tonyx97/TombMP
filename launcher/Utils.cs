using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Diagnostics;
using System.IO;
using System.Windows;
using System.Security.Cryptography;

namespace launcher
{
    static public class Utils
    {
        static public FileStream open_file(string filename, FileMode mask)
        {
            try { return File.Open(filename, mask); }
            catch { return null; }
        }

        static public byte[] read_file(string filename)
        {
            try { return File.ReadAllBytes(filename); }
            catch { return null; }
        }

        static public string hash_file(string filename)
        {
            var data = read_file(filename);
            if (data == null)
                return null;

            try
            {
                var hash = (new SHA512Managed()).ComputeHash(data);

                return BitConverter.ToString(hash).Replace("-", String.Empty).ToLower();
            }
            catch { return null; }
        }
    }
}
