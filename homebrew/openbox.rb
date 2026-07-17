class Openbox < Formula
  desc \"Highly configurable, next-generation stacking window manager\"
  homepage \"https://openbox.org/\"
  url \"https://github.com/openbox/openbox/releases/download/v4.0.0/openbox-4.0.0.tar.gz\"
  sha256 \"PLACEHOLDER_SHA256\"
  license \"GPL-2.0-or-later\"
  head \"https://github.com/openbox/openbox.git\", branch: \"main\"

  depends_on \"meson\" => :build
  depends_on \"ninja\" => :build
  depends_on \"pkg-config\" => :build
  depends_on \"rust\" => :build
  depends_on \"cairo\"
  depends_on \"glib\"
  depends_on \"pango\"
  depends_on \"libx11\"
  depends_on \"libxinerama\"
  depends_on \"libxrandr\"
  depends_on \"libxcursor\"
  depends_on \"libxft\"
  depends_on \"libxml2\"
  depends_on \"startup-notification\"
  depends_on \"python@3.11\"

  # XQuartz is required for X11 on macOS
  depends_on :x11 => :optional

  def install
    ENV[\"PYTHON\"] = which(\"python3.11\")

    # Disable Wayland on macOS (not supported)
    wayland_option = OS.mac? ? \"false\" : \"true\"

    system \"meson\", \"setup\", \"build\",
           \"-Dprefix=#{prefix}\",
           \"-Dbuildtype=release\",
           \"-Dportal=true\",
           \"-Dextensions=true\",
           \"-Dpython-bindings=true\",
           \"-Dwayland=#{wayland_option}\"

    system \"ninja\", \"-C\", \"build\"
    system \"ninja\", \"-C\", \"build\", \"install\"

    # Install Python bindings
    cd \"python\" do
      system \"python3.11\", \"-m\", \"pip\", \"install\", \".\", \"--prefix=#{prefix}\"
    end
  end

  def post_install
    # Create default config directory
    (var/\"openbox\").mkpath
  end

  def caveats
    <<~EOS
      Openbox requires an X11 server to run on macOS.

      If you haven't installed XQuartz:
        brew install --cask xquartz

      To start Openbox:
        openbox

      Configuration files are located at:
        ~/.config/openbox/

      For session management with display managers, see:
        #{opt_pkgshare}/xsessions/openbox.desktop
    EOS
  end

  service do
    run [opt_bin/\"openbox\", \"--replace\"]
    log_path var/\"log/openbox.log\"
    error_log_path var/\"log/openbox.log\"
    keep_alive true
  end

  test do
    system \"#{bin}/openbox\", \"--version\"
    system \"#{bin}/obctl\", \"--version\"
  end
end
