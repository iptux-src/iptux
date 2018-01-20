require 'formula'

class Iptux < Formula
  homepage 'https://github.com/iptux-src/iptux'
  url 'https://github.com/iptux-src/iptux/archive/v0.7.3.tar.gz'
  sha256 '287584507900a901984cff415c499fc3831c4041367b8332ab824798de1f4316'

  depends_on :x11
  depends_on 'gettext'
  depends_on 'gtk+'
  depends_on 'jsoncpp' 
  depends_on 'gstreamer'
  depends_on 'pkg-config' => :build
  depends_on 'cmake' => :build

  def install
    system "cmake", "-DCMAKE_INSTALL_PREFIX:PATH=#{prefix}", "."
    system "make", "install"
  end
end
