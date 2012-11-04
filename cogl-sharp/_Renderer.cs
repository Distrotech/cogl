/* This file has been generated by parse-gir.py, do not hand edit */
using System;
using System.Runtime.InteropServices;

namespace Cogl
{
    public partial class Renderer
    {
        [DllImport("cogl2.dll")]
        public static extern void cogl_renderer_add_constraint(IntPtr o, RendererConstraint constraint);

        public void AddConstraint(RendererConstraint constraint)
        {
            cogl_renderer_add_constraint(handle, constraint);
        }

        [DllImport("cogl2.dll")]
        public static extern Driver cogl_renderer_get_driver(IntPtr o);

        public Driver GetDriver()
        {
            return cogl_renderer_get_driver(handle);
        }

        [DllImport("cogl2.dll")]
        public static extern int cogl_renderer_get_n_fragment_texture_units(IntPtr o);

        public int GetNFragmentTextureUnits()
        {
            return cogl_renderer_get_n_fragment_texture_units(handle);
        }

        [DllImport("cogl2.dll")]
        public static extern WinsysID cogl_renderer_get_winsys_id(IntPtr o);

        public WinsysID GetWinsysId()
        {
            return cogl_renderer_get_winsys_id(handle);
        }

        [DllImport("cogl2.dll")]
        public static extern void cogl_renderer_remove_constraint(IntPtr o, RendererConstraint constraint);

        public void RemoveConstraint(RendererConstraint constraint)
        {
            cogl_renderer_remove_constraint(handle, constraint);
        }

        [DllImport("cogl2.dll")]
        public static extern void cogl_renderer_set_driver(IntPtr o, Driver driver);

        public void SetDriver(Driver driver)
        {
            cogl_renderer_set_driver(handle, driver);
        }

        [DllImport("cogl2.dll")]
        public static extern void cogl_renderer_set_winsys_id(IntPtr o, WinsysID winsys_id);

        public void SetWinsysId(WinsysID winsys_id)
        {
            cogl_renderer_set_winsys_id(handle, winsys_id);
        }

    }
}