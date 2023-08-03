#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0f));

    // --------------------------------------------------------------------
    // 1. inverse ile: model matrisi üzerindeki dönüşümleri geri alıyoruz. (rotation ve scale)
    // 2. transpose ile: bir önceki adımdaki rotasyon dönüşümünü geri alışımızı geri alıyoruz. (pure rotasyon(orthonormal) matrisler için transpose'la inverse aynı şey)
    // böylece orientation korunuyor.
    // 3. 4x4'ten 3x3'e matrise geçiyoruz (translation kısımları atılıyor)
    // çünkü per-vertex olarak tanımladığımız normal vektör'ler "direction vector", "position vector" değil. (homojen koordinatlarda w = 0)
    // dolayısıyla direction vektörlere translation uygulanamaz.
    // bu yüzden direk normal'i model matrisi ile çarparsak doğru sonuç alamayız.
    // dolayısıyla elimize geçen 3x3 matris'tete orientation bilgimiz mevcut sadece.
    // scale faktörümüz invert edildi, translation atıldı, orientation'ımız baştaki orientation. YEY!
    // --------------------------------------------------------------------
    Normal = mat3(transpose(inverse(model))) * aNormal;

    TexCoord = aTexCoord;

    gl_Position = projection * view * vec4(FragPos, 1.0f);
}