#include "app.h"
#include "temp.h"

int main(void)
{
    App app;

    if (!initApp(&app)) {
        return -1;
    }

    //mainLogika();

   
    app.data.isWhiteMove = true; 
    vec2 clickCursorPosLast{};
    vec2 totalOffset{};

    vec2 toField = {};
    vec2 fromField = {};

    int index = 0;
    int indexTOChangePiece = 0; 
    bool EscPress = false , EscPressClicked = false;
    bool isMenuOpen = false; 
    bool selectSlot = false; 
    bool confirmAction = false; 
    bool shouldClose = false; 
    bool shouldOpenPopUpToselectChessPiece = false; 

    glEnable(GL_DEPTH_TEST);

    while (!glfwWindowShouldClose(app.window) && !shouldClose) {

        glfwPollEvents();
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        glfwGetFramebufferSize(app.window, &app.SCR_WIDTH, &app.SCR_HEIGHT);
        glViewport(0, 0, app.SCR_WIDTH, app.SCR_HEIGHT);
 


        if (app.data.isMate) {

            ImGui::Begin("MATE"); 
            ImGui::Text("MATE"); 
            if (app.data.isWhiteMove) {
                ImGui::Text("BLACK WON!!!"); 
            }
            else {
                ImGui::Text("WHITE WON!!!"); 
            }
            ImGui::End(); 
        }


        ImGui::Begin("Move"); 
        if (app.data.isWhiteMove)
            ImGui::Text("White Move");
        else
            ImGui::Text("Black Move"); 
        ImGui::End(); 

        if (isMenuOpen) {
            ImGui::Begin("MENU", NULL, 0);
            if (ImGui::Button("RESTART")) {
                confirmAction = true; 
            }
            if (confirmAction) {
                ImGui::OpenPopup("My Dialog"); // Open the dialog popup
            }

            if (ImGui::BeginPopupModal("My Dialog", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::Text("Are you sure, you want to restart game?\nYour game will be lost.");
                if (ImGui::Button("YES")) {
                    confirmAction = false;
                    app.data.isWhiteMove = true; 
                    app.data.isMate = false; 
                    app.data.canMakeMove = true;
                    //RESTART GAME
                    for (int i = 0; i < ONE_COLOR_SIZE; i++) {
                        (*app.data.WhitePieces)[i] = createChessPiece({ positionsWhite[i].x - 4 + 0.5f, positionsWhite[i].y - 4 + 0.5f }, texturesIndexes[i], WHITE_CHESS_INDEX, app.scaleUI);
                        (*app.data.blackPieces)[i] = createChessPiece({ positionsBlack[i].x - 4 + 0.5f, positionsBlack[i].y - 4 + 0.5f }, texturesIndexes[i], BLACK_CHESS_INDEX, app.scaleUI);
                    }

                    glBindBuffer(GL_ARRAY_BUFFER, app.data.whitePiecesBuffers->VBO);
                    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(ChessPiece) * ONE_COLOR_SIZE, app.data.WhitePieces);
                    glBindBuffer(GL_ARRAY_BUFFER, app.data.blackPiecesBuffers->VBO);
                    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(ChessPiece) * ONE_COLOR_SIZE, app.data.blackPieces);
                    printf("RESTART\n"); 
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SameLine(); 
                if (ImGui::Button("NO")) {
                    //DO NOTHING
                    confirmAction = false;
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }

            ChoseSlotToLoadGame_PopUp("from what slot load chess game", &app.data);
            
            ChoseSlotToSaveGame_PopUp("to what slot save game?", app.data);

            if(ImGui::Button("Resume")){
                for (int i = 0; i < ONE_COLOR_SIZE; i++) {
                  
                    printf("piece: %f [] ", ((*app.data.WhitePieces)[i].vertecies->x/app.scaleUI) + 4.0f);
                    printf("%f \n", ((*app.data.WhitePieces)[i].vertecies->y / app.scaleUI) + 4.0f );
                }
                
                isMenuOpen = false;
            }
            if (ImGui::Button("Quit Game")) {
                shouldClose = true; 
            }
            ImGui::End();
        }

        EscPress = (glfwGetKey(app.window, GLFW_KEY_ESCAPE) == GLFW_PRESS);

        if (EscPress && !EscPressClicked){ //Press
            printf("OPEN MENU\n");
            EscPressClicked = true; 
            isMenuOpen = !isMenuOpen; 
        }
        else {
            if (EscPressClicked == true && EscPress == false) {
                //Release  
            }
            if (EscPress == false) EscPressClicked = false;
        }

        if (shouldOpenPopUpToselectChessPiece) {
            if (ChoseNewChessPiece_PopUp("na jaka figure zmienic pionka?", &(app.data))) {
                printf("end\n"); 
                updateChessPieceVBO(!app.data.isWhiteMove ? app.data.whitePiecesBuffers->VBO : app.data.blackPiecesBuffers->VBO, indexTOChangePiece, sizeof(ChessPiece), app.data.lastClickedPiece);
                shouldOpenPopUpToselectChessPiece = false; 
                app.data.canMakeMove = true; 
            }
        }

        float ascpectRatio = (float)app.SCR_WIDTH / (float)app.SCR_HEIGHT, t = 1.0f;
        glm::mat4 projection = glm::ortho(-ascpectRatio, ascpectRatio, -t, t);
        glBindBuffer(GL_UNIFORM_BUFFER, app.data.ProjectionUBO);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(projection), &projection);

        double xpos, ypos;
        glfwGetCursorPos(app.window, &xpos, &ypos);

        vec2 ndcCursor = { (xpos / app.SCR_WIDTH) * 2.0f - 1.0f,1.0f - (ypos / app.SCR_HEIGHT) * 2.0f };
        glm::vec4 cursorScreenCoords = glm::inverse(projection) * glm::vec4(ndcCursor.x, ndcCursor.y, 0.0f, 1.0f);
        app.data.cursorScreenCoords = { cursorScreenCoords.x, cursorScreenCoords.y };

        glm::vec4 boardscreencoords = glm::inverse(projection) * glm::vec4(-.0f, 0.0f, 0.0f, 1.0f);
        //boardscreencoords.x += 8 / 2 * app.scaleUI + 0.05;
        app.data.boardScreenCoords = { boardscreencoords.x, boardscreencoords.y };

        unsigned int currentVBO = app.data.isWhiteMove ? app.data.whitePiecesBuffers->VBO : app.data.blackPiecesBuffers->VBO; 
        unsigned int currentColorIndex = app.data.isWhiteMove ? WHITE_CHESS_INDEX : BLACK_CHESS_INDEX; 

        app.data.mouseBtnLeftPress = (glfwGetMouseButton(app.window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);

        if(!isMenuOpen && app.data.canMakeMove)
        if (app.data.mouseBtnLeftPress && !app.data.mouseBtnLeftPressClicked)
        {
            //printf("ON PRESS\n");
            app.data.mouseBtnLeftPressClicked = true;
            ChessPiece* clicked; 
            if (checkCursorHoverChessPieces(&app.data, &fromField, clicked, &index, currentColorIndex)) { 
                app.data.activePiece = clicked;
                app.data.lastClickedPiece = clicked; 
                bringFront(app.data.activePiece);
                updateChessPieceVBO(currentVBO, index, sizeof(ChessPiece), app.data.activePiece);

                totalOffset = { 0.0f, 0.0f }; 
            }
        }
        else {
            if (app.data.mouseBtnLeftPressClicked == true && app.data.mouseBtnLeftPress == false) {

                if (app.data.activePiece != nullptr)
                    checkCursorHover(app.data.boardLocalVert[0], 64, app.data.boardScreenCoords, app.data.cursorScreenCoords, &toField); 
                
                if(app.data.activePiece !=nullptr)
                if (!(toField.x == fromField.x && toField.y == fromField.y) && canBePlacedthere(toField.x, toField.y) ){
                    shouldOpenPopUpToselectChessPiece = false;

                    bringBack(app.data.activePiece);
                    updateChessPieceVBO(currentVBO, index, sizeof(ChessPiece), app.data.activePiece);

                    if (isMate()) {
                        app.data.isMate = true;
                        app.data.canMakeMove = false;
                    }

                    if (app.data.isWhiteMove && toField.y == 7 && app.data.activePiece->vertecies[0].pieceId == PAWN_TEXTURE_INDEX) {
                        printf("bialy doszedlem na osatnie pole chcl sie zmeinic\n");
                        shouldOpenPopUpToselectChessPiece = true; 
                        app.data.canMakeMove = false;
                        indexTOChangePiece = index;
                        
                        //Pop up
                    }
                    if (!app.data.isWhiteMove && toField.y == 0 && app.data.activePiece->vertecies[0].pieceId == PAWN_TEXTURE_INDEX) {
                        printf("czarny doszedlem na osatnie pole chcl sie zmeinic\n");
                        shouldOpenPopUpToselectChessPiece = true;
                        app.data.canMakeMove = false; 
                        indexTOChangePiece = index;
                        //Pop up
                    }

                    app.data.isWhiteMove = !app.data.isWhiteMove; 
                    vec2 posOnBoard = { toField.x - 4 + 0.5f, toField.y - 4 + 0.5f };
                    setChessPiecePosOnBoard(app.data.activePiece, posOnBoard, app.scaleUI);
                    updateChessPieceVBO(currentVBO, index, sizeof(ChessPiece), app.data.activePiece);

                    ChessPiece* hoveredPiece; 
                    if (checkCursorHoverChessPieces(&app.data, &toField, hoveredPiece, &index, app.data.isWhiteMove ? WHITE_CHESS_INDEX : BLACK_CHESS_INDEX)) {
                        setChessPiecePosOnBoard(hoveredPiece, { -5, 0}, app.scaleUI);
                        updateChessPieceVBO(app.data.isWhiteMove ? app.data.whitePiecesBuffers->VBO : app.data.blackPiecesBuffers->VBO, index, sizeof(ChessPiece), hoveredPiece);
                    }
                } else {
                    translateChessPiece(app.data.activePiece, { -totalOffset.x, -totalOffset.y }, &totalOffset);
                    bringBack(app.data.activePiece);
                    updateChessPieceVBO(currentVBO, index, sizeof(ChessPiece), app.data.activePiece);
                }
            }
            else {
                //printf("NO RELEASE"); 
            }
            if (app.data.mouseBtnLeftPress == false) app.data.mouseBtnLeftPressClicked = false;
        }
        if (!(app.data.mouseBtnLeftPress && app.data.activePiece != nullptr)) 
            app.data.activePiece = nullptr;

        if (app.data.activePiece != nullptr) {
            //MOVE LOGIC 
            vec2 offset = { app.data.cursorScreenCoords.x - clickCursorPosLast.x, app.data.cursorScreenCoords.y - clickCursorPosLast.y };
            translateChessPiece(app.data.activePiece, offset, &totalOffset);
            updateChessPieceVBO(currentVBO, index, sizeof(ChessPiece), app.data.activePiece);
        }
        clickCursorPosLast = app.data.cursorScreenCoords; 

        //rysowanie planszy <--->
        UseShaderProgram(app.data.boardShader);
        GLint uniformLocation = glGetUniformLocation(app.data.boardShader->programId, "transform");
        glm::mat4 transform = glm::mat4(1.0);
       
        transform = glm::mat4(1.0);
        transform = glm::translate(transform, glm::vec3(app.data.boardScreenCoords.x, app.data.boardScreenCoords.y, -1.0f));
        glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, glm::value_ptr(transform));
        glBindVertexArray(app.data.boardBuffers->VAO);
        glDrawElements(GL_TRIANGLES, (sizeof(*app.data.boardTriangleIndicies) / sizeof(int)), GL_UNSIGNED_INT, 0);


        //rysowanie pionek <--->
        UseShaderProgram(app.data.standard);
        uniformLocation = glGetUniformLocation(app.data.standard->programId, "transform");
        glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, glm::value_ptr(transform));
        glBindVertexArray(app.data.whitePiecesBuffers->VAO);
        glDrawElements(GL_TRIANGLES, sizeof(*app.data.whitePiecesIndicies)/sizeof(unsigned int), GL_UNSIGNED_INT, 0);

        glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, glm::value_ptr(transform));
        glBindVertexArray(app.data.blackPiecesBuffers->VAO);
        glDrawElements(GL_TRIANGLES, sizeof(*app.data.blackPiecesIndicies)/sizeof(unsigned int), GL_UNSIGNED_INT, 0);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(app.window);
    }

    terminateApp(&app); 

    return 0;
}
