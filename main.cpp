#include "raylib.h"
#include <iostream>
#include "math.h"

int randWithin(int start,int end){
    return start + rand()%(end-start);
}

class Sprite{
    Texture2D texture;
    Rectangle source; 
public:
    Sprite(const char* file){
        texture = LoadTexture(file);
        source.x = 0.0f;
        source.y = 0.0f;
        source.width = texture.width;
        source.height = texture.height;
    }

    void draw(int x, int y,int radius,float rotation, Color color=WHITE){
        Rectangle destination;
        destination.x = x;
        destination.y = y;
        destination.width = 2*radius;
        destination.height = 2*radius;
        Vector2 origin ;
        origin.x = radius;
        origin.y = radius;
        DrawTexturePro(texture,source,destination,origin,rotation,color);
    }
};



Sound hitSound, scoreSound;
Sprite* gemSprite;
Sprite* playerSprite;
Sprite* enemySprite;

void LoadAssets(){
    hitSound = LoadSound("assets/hit.mp3");
    scoreSound = LoadSound("assets/gem.wav");

    gemSprite = new Sprite("assets/gem3.png");
    playerSprite = new Sprite("assets/player2.png");
    enemySprite = new Sprite("assets/enemy2.png");
}

void UnloadAssets(){
    UnloadSound(hitSound);
    UnloadSound(scoreSound);
    delete gemSprite;
    delete playerSprite;
    delete enemySprite;
}

class CircularBody{
public :
    Vector2 position;
    int radius;

    bool collidesWith(CircularBody* other){
        int minDistance = radius + other->radius;
        int dx = position.x - other->position.x;
        int dy = position.y - other->position.y;
        int curDistance = sqrt(dx*dx + dy*dy);
        return curDistance <= minDistance;
    }
};


#define playerRadius 40
#define playerSpeed 10
class Player: public CircularBody{
public :
    int health = 5;
    int score = 0;

    Player(int x, int y){
        position.x = x;
        position.y = y;
        radius = playerRadius;
    }
    void Update() {
        if(IsKeyDown(KEY_A)) position.x -= playerSpeed;
        if(IsKeyDown(KEY_D)) position.x += playerSpeed;
        if(IsKeyDown(KEY_W)) position. y -= playerSpeed;
        if(IsKeyDown(KEY_S)) position. y += playerSpeed;   
    }
    void Draw(){
        playerSprite->draw(position.x,position.y,radius,0.0);
    }
};

#define maxEnemyRadius 60
#define minEnemyRadius 30 
#define rotationSpeed 5.0
#define minEnemySpeed 5
#define maxEnemySpeed 12

class Enemy: public CircularBody{
    float rotation = 0.0;
    float rotationDirection; 
    int speed;
public :
    Enemy(int x,int y){
        position.x = x;
        position.y = y;
        radius = randWithin(minEnemyRadius,maxEnemyRadius);
        if(rand()%2 == 0){
            rotationDirection = 1.0;
        }
        else rotationDirection = -1.0;
        speed = randWithin(minEnemySpeed,maxEnemySpeed);
    }

    void Update() {
        position.y += speed;
        rotation += rotationSpeed*rotationDirection;
    }
    void Draw(){
        enemySprite->draw(position.x,position.y,radius*1.3,rotation);
    }
};

#define gemRadius 25
class Gem: public CircularBody{
public :
    Gem(int x,int y){
        position.x = x;
        position.y = y;
        radius = gemRadius;
    }
    void Draw(){
        gemSprite->draw(position.x,position.y,radius,0.0);
    }
};

#define  maxEnemyCount  50

class Level{
public :
    int width;
    int height;
    int currentFrame;
    int enemySpawnInterval;

    Player* player;
    Enemy* enemies[maxEnemyCount];
    int enemyCount = 0;
    Gem* gem;


    Level(int _width, int _height){
        width = _width;
        height = _height;
        currentFrame = 0;
        enemySpawnInterval = 20;
        player = new Player(width/2,height - 15);
        gem = new Gem(
            randWithin(gemRadius,width-gemRadius),
            randWithin(height/3,height-gemRadius)
        );
    }

    void spawnEnemy(){
        if(enemyCount >= maxEnemyCount)return;
        int y = 0;
        int x = rand()% width;
        Enemy* enemy  = new Enemy(x,y);
        enemies[enemyCount] = enemy;
        enemyCount++;
    }

    void deSpawnEnemy(int enemyIndex){
        delete enemies[enemyIndex];
        enemies[enemyIndex] = enemies[enemyCount-1];
        enemyCount--;
    }

    void deSpawnEnemiesOutsideScreen(){

        for(int i = 0; i < enemyCount; i++){
            if(enemies[i]->position.y > height+enemies[i]->radius){
                deSpawnEnemy(i);
                i--;
            }
        }
    }

     
    void restrictWithinLevel(CircularBody* body){
        if(body->position.x < body->radius ) body->position.x = body->radius;
        if(body->position.x > width-body->radius) body->position.x = width-body->radius;
        if(body->position.y > height-body->radius) body->position.y = height-body->radius;
        if(body->position.y < body->radius) body->position.y = body->radius; 
    }

    void detectPlayerEnemyCollission(){
        for(int i = 0; i < enemyCount; i++){
            if(player->collidesWith(enemies[i])){
                player->health --;
                deSpawnEnemy(i);
                PlaySound(hitSound);
                
            }
        }
    }


    void relocateGem(){
        int maxAcrossWidth = width-(gem->radius);
        int maxAcrossHeight = height-(gem->radius);
        int minAcrossWidth = gem->radius;
        int minAcrossHeight = height/3;
        gem->position.x = randWithin(minAcrossWidth,maxAcrossWidth);
        gem->position.y = randWithin(minAcrossHeight,maxAcrossHeight);
    }

    /// returns whether the level needs to restart
    bool Play(){
        currentFrame++;

        
        if(player->health == 0){
            DrawText("Game Over!!",width/2-185,height/2-85,75,BLACK);
            DrawText(TextFormat("Score : %i",player->score),width/2 - 150,height/2,50,BLACK);
            DrawText("Press 'R' to restart the Game.",width/5,height/2+50,35,BLACK);
            DrawText("Press 'Q' to restart the Game.",width/5,height/2+85,35,BLACK);
            if(IsKeyPressed(KEY_R))return true;
            else if(IsKeyPressed(KEY_Q)) exit(0);
            else return false;
        }

        player->Update();
        restrictWithinLevel(player);
        for(int i = 0; i < enemyCount ; i++){
            enemies[i]->Update();
        }
        ///we spawn enemies in an interval,we are determining if it's the time to spawn an enemy.
        if(currentFrame%enemySpawnInterval == 0)spawnEnemy();
        deSpawnEnemiesOutsideScreen();

        ///simulate states
        if(player->collidesWith(gem)){
            relocateGem();
            player->score ++;
            PlaySound(scoreSound);
        }
        detectPlayerEnemyCollission();
        

        ///draw the level
        player->Draw();
        for(int i = 0; i < enemyCount ; i++){
            enemies[i]->Draw();
        }
        gem->Draw();
        const char* healthText = TextFormat("Health : %i",player->health);
        DrawText(healthText,20,20,30,BLACK);

        const char* scoreText = TextFormat("Score : %i",player->score);
        DrawText(scoreText,width-175,20,30,BLACK);
        return false;
    }

    ~Level(){
        delete player;
        for(int i=0; i<enemyCount; i++){
            delete enemies[i];
        }
        delete gem;
    }
};


int main() {
    srand(time(NULL));
    const int screenWidth = 800;
    const int screenHeight = 600;
    InitWindow(screenWidth, screenHeight, "Dodge&Earn");
    SetTargetFPS(60);
    InitAudioDevice();
    LoadAssets();

   
    Level* level = new Level(screenWidth,screenHeight);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(DARKGRAY);

        bool shouldRestart = level->Play();
        if(shouldRestart){
            delete level;
            level = new Level(screenWidth,screenHeight);
        }

        EndDrawing();
    }
    UnloadAssets();
    CloseAudioDevice();
    CloseWindow();
    return 0;
}
