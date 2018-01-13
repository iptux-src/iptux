require 'formula'

class Iptux < Formula
  homepage 'https://github.com/iptux-src/iptux'
  url 'https://github.com/iptux-src/iptux/archive/v0.7.1.tar.gz'
  sha256 'ea2af24e53ffd444a6c9693bba1de4a04d42cdc54fc4c513c15095a1ce395e00'

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
