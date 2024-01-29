#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>  // Thêm thư viện âm thanh
#include <time.h>
#include <list>
#include <sstream>
using namespace sf;

const int W = 1200;
const int H = 800;

float DEGTORAD = 0.017453f;

class Animation
{
public:
    float Frame, speed;
    Sprite sprite;
    std::vector<IntRect> frames;

    Animation() {}

    Animation(Texture& t, int x, int y, int w, int h, int count, float Speed)
    {
        Frame = 0;
        speed = Speed;

        for (int i = 0; i < count; i++)
            frames.push_back(IntRect(x + i * w, y, w, h));

        sprite.setTexture(t);
        sprite.setOrigin(w / 2, h / 2);
        sprite.setTextureRect(frames[0]);
    }

    void update()
    {
        Frame += speed;
        int n = frames.size();
        if (Frame >= n)
            Frame -= n;
        if (n > 0)
            sprite.setTextureRect(frames[int(Frame)]);
    }

    bool isEnd()
    {
        return Frame + speed >= frames.size();
    }
};

class Entity
{
public:
    float x, y, dx, dy, R, angle;
    bool life;
    std::string name;
    Animation anim;

    Entity() : life(true) {}

    void settings(Animation& a, int X, int Y, float Angle = 0, int radius = 1)
    {
        anim = a;
        x = X;
        y = Y;
        angle = Angle;
        R = radius;
    }

    virtual void update() {}

    void draw(RenderWindow& app)
    {
        anim.sprite.setPosition(x, y);
        anim.sprite.setRotation(angle + 90);
        app.draw(anim.sprite);

        CircleShape circle(R);
        circle.setFillColor(Color(255, 0, 0, 170));
        circle.setPosition(x, y);
        circle.setOrigin(R, R);
        //app.draw(circle);
    }

    virtual ~Entity() {}
};

class asteroid : public Entity
{
public:
    float speed;

    asteroid()
    {
        dx = rand() % 8 - 4;
        dy = rand() % 8 - 4;
        name = "asteroid";
        speed = 0.1; // Giảm tốc độ bay của asteroid
    }

    void update()
    {
        x += dx * speed;
        y += dy * speed;

        if (x > W)
            x = 0;
        if (x < 0)
            x = W;
        if (y > H)
            y = 0;
        if (y < 0)
            y = H;
    }
};

class bullet : public Entity
{
public:
    bullet() { name = "bullet"; }

    void update()
    {
        dx = cos(angle * DEGTORAD) * 6;
        dy = sin(angle * DEGTORAD) * 6;
        x += dx;
        y += dy;

        if (x > W || x < 0 || y > H || y < 0)
            life = false;
    }
};

class player : public Entity
{
public:
    bool thrust;
    int score;

    player() : score(0) { name = "player"; }

    void update()
    {
        if (thrust)
        {
            dx += cos(angle * DEGTORAD) * 0.2;
            dy += sin(angle * DEGTORAD) * 0.2;
        }
        else
        {
            dx *= 0.99;
            dy *= 0.99;
        }

        int maxSpeed = 5;
        float currentSpeed = sqrt(dx * dx + dy * dy);
        if (currentSpeed > maxSpeed)
        {
            dx *= maxSpeed / currentSpeed;
            dy *= maxSpeed / currentSpeed;
        }

        x += dx;
        y += dy;

        if (x > W)
            x = 0;
        if (x < 0)
            x = W;
        if (y > H)
            y = 0;
        if (y < 0)
            y = H;
    }
};

bool isCollide(Entity* a, Entity* b)
{
    return (b->x - a->x) * (b->x - a->x) +
        (b->y - a->y) * (b->y - a->y) <
        (a->R + b->R) * (a->R + b->R);
}

int main()
{
    srand(time(0));

    RenderWindow app(VideoMode(W, H), "Asteroids!");
    app.setFramerateLimit(60);

    // Load âm nhạc
    Music music;
    if (!music.openFromFile("Mission-Impossible.ogg"))
    {
        // Xử lý lỗi khi không thể load âm nhạc
        return EXIT_FAILURE;
    }
    music.setLoop(true); // Lặp lại âm nhạc
    music.play();       // Phát âm nhạc

    Texture t1, t2, t3, t4, t5, t6, t7;
    t1.loadFromFile("images/spaceship.png");
    t2.loadFromFile("images/background.jpg");
    t3.loadFromFile("images/explosions/type_C.png");
    t4.loadFromFile("images/rock.png");
    t5.loadFromFile("images/fire_blue.png");
    t6.loadFromFile("images/rock_small.png");
    t7.loadFromFile("images/explosions/type_B.png");

    t1.setSmooth(true);
    t2.setSmooth(true);

    Sprite background(t2);

    Animation sExplosion(t3, 0, 0, 256, 256, 48, 0.5);
    Animation sRock(t4, 0, 0, 64, 64, 16, 0.2);
    Animation sRock_small(t6, 0, 0, 64, 64, 16, 0.2);
    Animation sBullet(t5, 0, 0, 32, 64, 16, 0.8);
    Animation sPlayer(t1, 40, 0, 40, 40, 1, 0);
    Animation sPlayer_go(t1, 40, 40, 40, 40, 1, 0);
    Animation sExplosion_ship(t7, 0, 0, 192, 192, 64, 0.5);

    std::list<Entity*> entities;

    for (int i = 0; i < 15; i++)
    {
        asteroid* a = new asteroid();
        a->settings(sRock, rand() % W, rand() % H, rand() % 360, 25);
        entities.push_back(a);
    }

    player* p = new player();
    p->settings(sPlayer, 200, 200, 0, 20);
    entities.push_back(p);

    /////main loop/////
    while (app.isOpen())
    {
        Event event;
        while (app.pollEvent(event))
        {
            if (event.type == Event::Closed)
                app.close();

            if (event.type == Event::KeyPressed)
                if (event.key.code == Keyboard::Space)
                {
                    bullet* b = new bullet();
                    b->settings(sBullet, p->x, p->y, p->angle, 10);
                    entities.push_back(b);
                }
        }

        if (p->score == 0)
        {
            // Nếu điểm số bằng 0, reset tốc độ của tất cả các asteroid
            for (auto i : entities)
            {
                if (i->name == "asteroid")
                {
                    asteroid* a = dynamic_cast<asteroid*>(i);
                    if (a)
                    {
                        a->speed = 0.1; // Giá trị tốc độ ban đầu của asteroid
                    }
                }
            }
        }

        if (Keyboard::isKeyPressed(Keyboard::Right))
            p->angle += 3;
        if (Keyboard::isKeyPressed(Keyboard::Left))
            p->angle -= 3;
        if (Keyboard::isKeyPressed(Keyboard::Up))
            p->thrust = true;
        else
            p->thrust = false;

        // Tăng dần tốc độ bay của asteroid dựa trên điểm số
        for (auto i : entities)
        {
            if (i->name == "asteroid")
            {
                asteroid* a = dynamic_cast<asteroid*>(i);
                if (a)
                {
                    a->speed += p->score * 0.0001;
                }
            }
        }

        for (auto a : entities)
            for (auto b : entities)
            {
                if (a->name == "asteroid" && b->name == "bullet")
                    if (isCollide(a, b))
                    {
                        a->life = false;
                        b->life = false;
                        p->score += 10;

                        Entity* e = new Entity();
                        e->settings(sExplosion, a->x, a->y);
                        e->name = "explosion";
                        entities.push_back(e);

                        for (int i = 0; i < 2; i++)
                        {
                            if (a->R == 15)
                                continue;
                            Entity* e = new asteroid();
                            e->settings(sRock_small, a->x, a->y, rand() % 360, 15);
                            entities.push_back(e);
                        }
                    }

                if (a->name == "player" && b->name == "asteroid")
                    if (isCollide(a, b))
                    {
                        b->life = false;

                        Entity* e = new Entity();
                        e->settings(sExplosion_ship, a->x, a->y);
                        e->name = "explosion";
                        entities.push_back(e);

                        p->score = 0;

                        p->settings(sPlayer, W / 2, H / 2, 0, 20);
                        p->dx = 0;
                        p->dy = 0;
                    }
            }

        if (p->thrust)
            p->anim = sPlayer_go;
        else
            p->anim = sPlayer;

        for (auto e : entities)
            if (e->name == "explosion")
                if (e->anim.isEnd())
                    e->life = 0;

        if (rand() % 150 == 0)
        {
            asteroid* a = new asteroid();
            a->settings(sRock, 0, rand() % H, rand() % 360, 25);
            entities.push_back(a);
        }

        for (auto i = entities.begin(); i != entities.end();)
        {
            Entity* e = *i;

            e->update();
            e->anim.update();

            if (e->life == false)
            {
                i = entities.erase(i);
                delete e;
            }
            else
                i++;
        }

        //////draw//////
        app.draw(background);

        Font font;
        font.loadFromFile("arial.ttf");
        Text text;
        text.setFont(font);
        text.setString("Score: " + std::to_string(p->score));
        text.setCharacterSize(24);
        text.setFillColor(Color::White);
        text.setPosition(10, 10);
        app.draw(text);

        for (auto i : entities)
            i->draw(app);

        app.display();
    }

    return 0;
}
