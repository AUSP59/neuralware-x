# SPDX-License-Identifier: Apache-2.0
class Nwx < Formula
  desc "NEURALWARE-X core C++ CLI"
  homepage "https://example.com/nwx"
  url "https://example.com/nwx/releases/download/v0.1.0/nwx-0.1.0-linux-x86_64.tar.gz"
  version "0.1.0"
  license "Apache-2.0"

  def install
    bin.install "bin/nwx"
  end

  test do
    system "#{bin}/nwx", "--help"
  end
end
