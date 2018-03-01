require 'formula'

class Iptux < Formula
  head 'https://github.com/iptux-src/iptux.git'
  homepage 'https://github.com/iptux-src/iptux'
  url 'https://github.com/iptux-src/iptux/archive/v0.7.4.tar.gz'
  sha256 'cb208c54f9e888b40aed6c9590dcf25bfdcceb1a5a964b57220ede0fa816f22a'

  depends_on :x11 => :optional
  depends_on 'gettext'
  depends_on 'gtk+' unless build.head?
  depends_on 'gtk+3' if build.head?
  depends_on 'jsoncpp'
  depends_on 'gstreamer' => :optional
  depends_on 'gst-plugins-base' => ["with-ogg", "with-libvorbis"] if build.with? "gstreamer"
  depends_on 'gst-plugins-good' if build.with? "gstreamer"
  depends_on 'pkg-config' => :build
  depends_on 'cmake' => :build
  unless OS.mac?
    depends_on "linuxbrew/xorg/xorg"
  end

  def install
    system "cmake", "-DCMAKE_INSTALL_PREFIX:PATH=#{prefix}", "."
    system "make", "install"
  end
end
