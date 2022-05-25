// Dear ImGui: standalone example application for GLFW + OpenGL 3, using programmable pipeline
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_stdlib.h"
//#include <stdio.h>
#include <enet/enet.h>
//#include <sys/types.h>
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

#include "../protocol.h"
//#include <stdio.h>
#include <windows.h>
#include <process.h>


// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

static int portLobby = 10887;
static std::vector<Room> rooms;
static std::vector<User> users;
static User user;
static Room room;
static AgarSettings agarSettings;
static CarsSettings carsSettings;

bool show_login_window = true;
bool show_rooms_list_window = false;
bool show_create_room_window = false;
bool show_room_window = false;
bool show_waiting = false;

void enet_loop(ENetHost* client)
{
  ENetEvent event;
  while (enet_host_service(client, &event, 0) > 0)
  {
    switch (event.type)
    {
    case ENET_EVENT_TYPE_CONNECT:
      printf("Connection with %x:%u established\n", event.peer->address.host, event.peer->address.port);
      break;
    case ENET_EVENT_TYPE_RECEIVE:
      switch (get_packet_type(event.packet))
      {
      case E_LOBBY_SERVER_TO_LOBBY_CLIENT_USER_ID:
      {
        deserialize_user_id(event.packet, user.id);
        send_refresh(event.peer);
        break;
      }
      case E_LOBBY_SERVER_TO_LOBBY_CLIENT_ROOM_ID:
      {
        deserialize_room_id(event.packet, room.id);
        show_waiting = false;
        show_room_window = true;
        break;
      }
      case E_LOBBY_SERVER_TO_LOBBY_CLIENT_ROOMS_LIST:
      {
        rooms.clear();
        deserialize_rooms_list(event.packet, rooms);
        printf("rooms size %ld\n", rooms.size());
        break;
      }
      case E_LOBBY_SERVER_TO_LOBBY_CLIENT_ROOM_INFO_AGAR:
      {
        users.clear();
        deserialize_room_info_agar(event.packet, room, agarSettings, users);
        show_waiting = false;
        show_room_window = true;
        break;
      }
      case E_LOBBY_SERVER_TO_LOBBY_CLIENT_ROOM_INFO_CARS:
      {
        users.clear();
        deserialize_room_info_cars(event.packet, room, carsSettings, users);
        show_waiting = false;
        show_room_window = true;
        break;
      }
      case E_LOBBY_SERVER_TO_LOBBY_CLIENT_USERS_LIST:
      {
        users.clear();
        deserialize_users_list(event.packet, users);
        break;
      }
      case E_LOBBY_SERVER_TO_LOBBY_CLIENT_SERVER_INFO:
      {
        uint16_t port;
        deserialize_server_port(event.packet, port);
        printf("try launch, port: %d", port);
        printf("modif: %f", agarSettings.speedModif);
        if (room.type == 0)
        {
          const std::string path = "../../../agario/x64/Debug/hw-6.exe";
          _execl(path.c_str(), "go", std::to_string(port).c_str(),
                 std::to_string(agarSettings.speedModif).c_str(), nullptr);
        }
        else
        {
          const std::string path = "../../../cars/x64/Debug/hw-6.exe";
          _execl(path.c_str(), "go", std::to_string(port).c_str(),
                 std::to_string(carsSettings.forwardAccel).c_str(),
                 std::to_string(carsSettings.breakAccel).c_str(),
                 std::to_string(carsSettings.speedRotation).c_str(), nullptr);
        }
        break;
      }
      };
      printf("Packet received '%s'\n", event.packet->data);
      break;
    default:
      break;
    };
  }
}

void ui_loop(ENetPeer* server)
{
  if (show_login_window)
  {
    ImGui::Begin("Login Window");
    ImGui::InputText("Write your name", user.name, 32);
    if (ImGui::Button("Login") && user.name[0] != '\0')
    {
      send_login(server, user);
      show_login_window = false;
      show_rooms_list_window = true;
    }
    ImGui::End();
  }

  if (show_waiting)
  {
    ImGuiWindowFlags window_flags = 0;

    window_flags |= ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoResize;
    window_flags |= ImGuiWindowFlags_NoCollapse;
    window_flags |= ImGuiWindowFlags_NoTitleBar;

    ImGui::Begin("test", 0, window_flags);
    ImGui::Text("Waiting...");
    ImGui::End();
  }

  if (show_rooms_list_window)
  {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::Begin("Rooms list");
    static int selected = -1;
    if (ImGui::BeginTable("Rooms list", 4))
    {
      ImGui::TableSetupColumn("Name");
      ImGui::TableSetupColumn("Players");
      ImGui::TableSetupColumn("Type");
      ImGui::TableSetupColumn("Running");
      ImGui::TableHeadersRow();
      for (const auto& room : rooms)
      {
        
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGuiSelectableFlags selectable_flags = ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap;
        if (ImGui::Selectable(room.name, room.id == selected, selectable_flags))
        {
          selected = room.id;
        }
        ImGui::TableNextColumn();
        ImGui::Text("%d/%d", room.curPlayers, room.maxPlayers);
        ImGui::TableNextColumn();
        ImGui::Text(room.type == 0 ? "Agario" : "Cars");
        ImGui::TableNextColumn();
        ImGui::Text(room.running == 0 ? "Waiting" : "Running");
      }
      ImGui::EndTable();
    }
    if (selected != -1)
    {
      for (int i = 0; i < rooms.size(); ++i)
      {
        if (rooms[i].id == selected)
        {
          room = rooms[i];
        }
      }      
    }
    
    if (ImGui::Button("Create room"))
    {
      show_rooms_list_window = false;
      show_create_room_window = true;
      strcpy(room.name, "");
    }
    ImGui::SameLine();
    if (ImGui::Button("Refresh"))
    {
      printf("send refresh\n");
      send_refresh(server);
    }
    ImGui::SameLine();
    if (ImGui::Button("Join") && selected != -1)
    {
      show_rooms_list_window = false;
      show_waiting = true;
      send_join(server, user.id, room.id);
    }
    ImGui::End();
  }

  if (show_create_room_window)
  {
    constexpr float buttonSize = 50.0f;
    ImGui::Begin("Create room");
    auto windowSize = ImGui::GetWindowSize();
    static int type = 0;
    static int maxPlayers = 8;
    ImGui::Text("Window size: %f, %f", windowSize.x, windowSize.y);
    ImGui::InputText("Room name", room.name, 32);
    ImGui::Combo("Type", &type, "Agar\0Cars\0\0");
    ImGui::SliderInt("Max players", &maxPlayers, 1, 16);
    room.type = static_cast<uint8_t>(type);
    room.maxPlayers = static_cast<uint8_t>(maxPlayers);

    if (type == 0) //Agar
    {
      static int botsCount;
      static float minStartRadius = 0.1f;
      static float maxStartRadius = 1.0f;
      static float weightLoss = 0.5f;
      static float speedModif = 1.0f;
      ImGui::Text("Agar settings:");
      ImGui::SliderInt("Bot's count", &botsCount, 0, 32);
      ImGui::SliderFloat("Min start radius", &minStartRadius, 0.0f, 3.0f, "%.1f");
      ImGui::SliderFloat("Max start radius", &maxStartRadius, 0.0f, 3.0f, "%.1f");
      ImGui::SliderFloat("Weight loss", &weightLoss, 0.0f, 1.0f, "%.1f");
      ImGui::SliderFloat("Speed modificator", &speedModif, 0.1f, 10.0f, "%.1f");
      agarSettings.botsCount = static_cast<uint16_t>(botsCount);
      agarSettings.minStartRadius = static_cast<float>(minStartRadius);
      agarSettings.maxStartRadius = static_cast<float>(maxStartRadius);
      agarSettings.weightLoss = static_cast<float>(weightLoss);
      agarSettings.speedModif = static_cast<float>(speedModif);
    }
    else if (type == 1) //Cars
    {
      static float forwardAccel = 12.0f;
      static float breakAccel = 3.0f;
      static float speedRotation = 0.3f;
      ImGui::Text("Cars settings:");
      ImGui::SliderFloat("Forward accel", &forwardAccel, 0.0f, 30.0f, "%.1f");
      ImGui::SliderFloat("Break accel", &breakAccel, 0.0f, 30.0f, "%.1f");
      ImGui::SliderFloat("Speed rotation", &speedRotation, 0.0f, 1.0f, "%.1f");
      carsSettings.forwardAccel = forwardAccel;
      carsSettings.breakAccel = breakAccel;
      carsSettings.speedRotation = speedRotation;
    }
    else
    {
      ImGui::Text("Something was wrong");
    }
    if (ImGui::Button("Create", { buttonSize , 0 }))
    {
      show_create_room_window = false;
      show_waiting = true;
      //users.push_back(user);
      if (room.type == 0)
      {
        send_create_agar_room(server, room, agarSettings);
      }
      else if (room.type == 1)
      {
        send_create_cars_room(server, room, carsSettings);
      }
    }
    ImGui::SameLine(windowSize.x - buttonSize - 10.0f);
    if (ImGui::Button("Cancel"))
    {
      show_create_room_window = false;
      show_rooms_list_window = true;
      send_refresh(server);
    }
    ImGui::End();
  }

  if (show_room_window)
  {
    ImGui::Begin("Room");
    ImGui::Text("Room info: %s players %d/%d", room.name, users.size(), room.maxPlayers);
    if (room.type == 0)
    {
      ImGui::Text("Mode: agario. Params: botsCount = %d, minStartRadius = %f, maxStartRadius = %f, weightLoss = %f, speedModif = %f",
                  agarSettings.botsCount, agarSettings.minStartRadius, agarSettings.maxStartRadius, 
                  agarSettings.weightLoss, agarSettings.speedModif);
    }
    else
    {
      ImGui::Text("Mode: cars. Params: forwardAccel = %f, breakAccel = %f, speedRotation = %f",
        carsSettings.forwardAccel, carsSettings.breakAccel, carsSettings.speedRotation);
    }
    ImGui::Text("Players list:");
    static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
    static bool display_headers = false;
    if (ImGui::BeginTable("table1", 1, flags))
    {
      for (const auto& user : users)
      {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text(user.name);
      }
      ImGui::EndTable();
    }
    
    if (ImGui::Button("Leave"))
    {
      //send req about leaving from room
      send_leave(server, user.id, room.id);
      send_refresh(server);
      show_room_window = false;
      show_rooms_list_window = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Start"))
    {
      send_start(server, room.id);
    }
    ImGui::End();
  }
}

static void glfw_error_callback(int error, const char* description)
{
  fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

int main(int, char**)
{
  if (enet_initialize() != 0)
  {
    printf("Cannot init ENet");
    return 1;
  }
  
  ENetHost* client = enet_host_create(nullptr, 1, 2, 0, 0);
  if (!client)
  {
    printf("Cannot create ENet client\n");
    return 1;
  }

  ENetAddress address;
  enet_address_set_host(&address, "localhost");
  address.port = portLobby;

  ENetPeer* serverPeer = enet_host_connect(client, &address, 2, 0);
  if (!serverPeer)
  {
    printf("Cannot connect to server");
    return 1;
  }
  // Setup window
  glfwSetErrorCallback(glfw_error_callback);
  if (!glfwInit())
    return 1;

  // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
  const char* glsl_version = "#version 100";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    // GL 3.2 + GLSL 150
  const char* glsl_version = "#version 150";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
  const char* glsl_version = "#version 130";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
  //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    // Create window with graphics context
  GLFWwindow* window = glfwCreateWindow(1280, 720, "Lobby-client", NULL, NULL);
  if (window == NULL)
    return 1;
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1); // Enable vsync

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO(); (void)io;
  //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
  //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();
  //ImGui::StyleColorsClassic();

  // Setup Platform/Renderer backends
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(glsl_version);

  // Load Fonts
  // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
  // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
  // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
  // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
  // - Read 'docs/FONTS.md' for more instructions and details.
  // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
  //io.Fonts->AddFontDefault();
  //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
  //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
  //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
  //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
  //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
  //IM_ASSERT(font != NULL);

  // Our state
  bool show_demo_window = true;
  bool show_another_window = false;
  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

  // Main loop
  while (!glfwWindowShouldClose(window))
  {
    enet_loop(client);

    // Poll and handle events (inputs, window resize, etc.)
    // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
    // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
    // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
    // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
    glfwPollEvents();

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
    if (show_demo_window)
      ImGui::ShowDemoWindow(&show_demo_window);

    // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
    {
      static float f = 0.0f;
      static int counter = 0;

      ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

      ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
      ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
      ImGui::Checkbox("Another Window", &show_another_window);

      ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
      ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

      if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
        counter++;
      ImGui::SameLine();
      ImGui::Text("counter = %d", counter);

      ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
      ImGui::End();
    }

    // 3. Show another simple window.
    if (show_another_window)
    {
      ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
      ImGui::Text("Hello from another window!");
      if (ImGui::Button("Close Me"))
        show_another_window = false;
      ImGui::End();
    }

    ui_loop(serverPeer);

    // Rendering
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
  }

  // Cleanup
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
  }
