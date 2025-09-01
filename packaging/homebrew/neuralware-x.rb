# SPDX-License-Identifier: Apache-2.0
class NeuralwareX < Formula
  desc "NEURALWARE-X core"
  homepage "https://example.com"
  url "https://github.com/your-org/neuralware-x/archive/refs/tags/v1.0.0.tar.gz"
  sha256 "<fill-me>"
  license "Apache-2.0"

  depends_on "cmake" => :build
  depends_on "ninja" => :build

  def install
    system "cmake", "-S", ".", "-B", "build", "-G", "Ninja", "-DCMAKE_BUILD_TYPE=Release"
    system "cmake", "--build", "build"
    system "cmake", "--install", "build", "--prefix", prefix
  end

  test do
    system "#{bin}/nwx_serve", "--help"
  end
end
