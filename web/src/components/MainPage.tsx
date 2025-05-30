import { Box, Toolbar } from "@mui/material";
import SideBar from "./SideBar.tsx";
import AppNavbar from "./AppNavbar.tsx";
import { Outlet } from "react-router";
import { useState } from "react";
import SettingsChangedWidget from "./atoms/SettingsChangedWidget.tsx";
import ErrorLogWidget from "./ErrorLogWidget.tsx";
import Stack from "@mui/material/Stack";

const drawerWidth = 240;

export default function MainPage() {
  const [mobileOpen, setMobileOpen] = useState(false);
  const [isClosing, setIsClosing] = useState(false);

  const handleDrawerClose = () => {
    setIsClosing(true);
    setMobileOpen(false);
  };
  const handleDrawerTransitionEnd = () => {
    setIsClosing(false);
  };

  const handleDrawerToggle = () => {
    if (!isClosing) {
      setMobileOpen(!mobileOpen);
    }
  };
  return (
    <Box sx={{ display: "flex" }}>
      <AppNavbar handleDrawerToggle={handleDrawerToggle} />
      <SideBar mobileOpen={mobileOpen} onClose={handleDrawerClose} onTransitionEnd={handleDrawerTransitionEnd} />

      <Box
        component="main"
        sx={{
          flexGrow: 1,
          p: { xs: 0, sm: 3 },
          width: { sm: `calc(100% - ${drawerWidth}px)` },
        }}
      >
        <Toolbar />
        <Stack direction="row" spacing={2} sx={{ justifyContent: "center" }}>
          <SettingsChangedWidget />
          <ErrorLogWidget />
        </Stack>
        <Outlet />
      </Box>
    </Box>
  );
}
