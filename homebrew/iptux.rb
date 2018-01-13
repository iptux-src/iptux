require 'formula'

class Iptux < Formula
  homepage 'https://github.com/iptux-src/iptux'
  url 'https://github.com/iptux-src/iptux/archive/v0.7.0.tar.gz'
  sha256 '559c6c7f35e1379d20cf24d7f03a413b1f4ec0f5be329913a8b8db1aec5a2b66'

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
