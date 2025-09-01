# SPDX-License-Identifier: Apache-2.0
class Neuralwarex < Formula
  desc "Verifiable neural networks in C++"
  homepage "https://example.org/neuralwarex"
  url "https://github.com/yourorg/neuralwarex/archive/refs/tags/v0.2.0.tar.gz"
  sha256 "REPLACE_WITH_REAL_SHA256"
  license "Apache-2.0"

  depends_on "cmake" => :build

  def install
    system "cmake", "-S", ".", "-B", "build", *std_cmake_args
    system "cmake", "--build", "build"
    system "cmake", "--install", "build"
  end

  test do
    system "#{bin}/nwx", "--help"
  end
end
