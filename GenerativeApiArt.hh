<?hh

class GenerativeApiArt {

  private int $size;
  private string $api_key = "";

  private string $url = "";
  private string $safe_url = "";
  private string $raw_string = "";
  private ?object $json = null;
  private string $author = "";
  private string $title = "";
  private string $real_image_url = "";
  private mixed $intermediary_real_image;
  private mixed $real_image;
  private array<string,int> $color = array();
  private string $generated_image = "";

  const int MODE_BITMAP = 0;
  const int MODE_COLOR = 1;

  const int MAX_WORKS = 308616;

  public function __construct(string $key, int $size = 500)
  {
    $this->size = $size;
    $this->api_key = $key;
  }

  public function call(): this
  {

    $page = mt_rand(1,static::MAX_WORKS);

    $this->url = "http://api.art.rmngp.fr/v1/works/suggested?per_page=1&page=" . $page . "&exists=name%2Ctitle&api_key=" . $this->api_key;
    $this->safe_url = "http://api.art.rmngp.fr/v1/works/suggested?per_page=1&page=" . $page . "&exists=name%2Ctitle&api_key=demo";

    $ch = curl_init();
    curl_setopt($ch, CURLOPT_FOLLOWLOCATION, 1);
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
    curl_setopt($ch, CURLOPT_URL, $this->url);
    $result = curl_exec($ch);

    curl_close($ch);

    $this->raw_string = trim(preg_replace('/\s+/', ' ', $result));
    $json = json_decode($result);


    if ($json === null){
      $this->json = null;
    } else {
      $this->json = $json;
    }

    return $this;

  }

  public function extract(): this
  {
    
    if ($this->json) {
      $hit = $this->json->hits->hits[0];

      if (count($hit->_source->authors) > 0){
        $this->author = $hit->_source->authors[0]->name->fr;
      } else {
        $this->author = "(Unknown)";
      }

      $this->title = $hit->_source->title->fr;

      if (count($hit->_source->images) > 0){
        $this->real_image_url = $hit->_source->images[0]->urls->large->url;

        $this->intermediary_real_image = imagecreatefromjpeg($this->real_image_url);
        $width = imagesx($this->intermediary_real_image);
        $height = imagesy($this->intermediary_real_image);
        $pixel = imagecreatetruecolor(1, 1);
        imagecopyresampled($pixel, $this->intermediary_real_image, 0, 0, 0, 0, 1, 1, $width, $height);

        // extract medium color
        $rgb = imagecolorat($pixel, 0, 0);
        $this->color = imagecolorsforindex($pixel, $rgb);

        $this->real_image = imagecreatetruecolor(500, 500);
        imagecopyresampled($this->real_image, $this->intermediary_real_image, 0, 0, 0, 0, 500, 500, $width, $height);
      }
    }

    return $this;

  }

  public function create(int $mode): void
  {

    $this->generated_image = imagecreatetruecolor($this->size, $this->size);
    $current = $this->raw_string;
    $l = strlen($current);

    if ($mode === static::MODE_COLOR) {

      for ($x=0; $x < $this->size; $x++) {

        for ($y=0; $y < $this->size; $y++) {

          $current_char = $current[($x*$this->size + $y) % $l];
          $val = ord($current_char);

          $color = imagecolorallocate($this->generated_image, ($this->color['red'] + $val) % 255, ($this->color['green'] + $val) % 255, ($this->color['blue'] + $val) % 255);

          imagesetpixel($this->generated_image, $x*2, $y*2, $color);
          imagesetpixel($this->generated_image, $x*2+1, $y*2, $color);
          imagesetpixel($this->generated_image, $x*2, $y*2+1, $color);
          imagesetpixel($this->generated_image, $x*2+1, $y*2+1, $color);

        }

      }

    } else if ($mode === static::MODE_BITMAP){

      $width = imagesx($this->real_image);
      $height = imagesy($this->real_image);

      for ($x=0; $x < $this->size; $x++) {

        for ($y=0; $y < $this->size; $y++) {

          $current_char = $current[($x*$this->size + $y) % $l];
          $val = intval((ord($current_char) - 96));
          if ($x < $width && $y < $height) {
            $real_color = imagecolorat($this->real_image, $x, $y);
            $real_color = imagecolorsforindex($this->real_image, $real_color);
          } else {
            $real_color = imagecolorallocate($this->real_image, 0,0,0);
          }

          $color = imagecolorallocate($this->generated_image, ($real_color['red'] + $val) % 255, ($real_color['green'] + $val) % 255, ($real_color['blue'] + $val) % 255);
          
          imagesetpixel($this->generated_image, $x, $y, $color);

        }

      }
    }

  }

  public function output(): void
  {

    // Output in a buffer :
    ob_start();
      imagepng($this->generated_image);
      $image = ob_get_contents();
    ob_end_clean();

    ob_start();
      imagepng($this->real_image);
      $real_image = ob_get_contents();
    ob_end_clean();

    imagedestroy($this->generated_image);
    imagedestroy($this->real_image);
  
    // Output a very simple page in HTML
    echo '<html><head><link href="http://fonts.googleapis.com/css?family=Lato:100,300" rel="stylesheet" type="text/css"><link href="https://maxcdn.bootstrapcdn.com/bootstrap/3.3.4/css/bootstrap.min.css" rel="stylesheet"><style>h1 {font-family: "Lato", sans-serif; text-align: center;} .header{ width:100%; border-bottom:1px solid #BBB; padding: 10px 20px;background: #EEE; margin-bottom: 50px; font-family: "Lato", sans-serif;} .clip {clip-path:polygon(0 0,' . $this->size . 'px 0,' . $this->size . 'px 100%, 0 100%); height:' . $this->size .'px; -webkit-clip-path:polygon(0 0,' . $this->size . 'px 0,' . $this->size . 'px 100%, 0 100%);}.images{display: block; width: 1010px; margin: auto; margin-top:50px;} .well{margin:auto; margin-top: 40px; width: 1000px;} .container{width:500px; height:500px; overflow:hidden;vertical-align:top;display: inline-block;margin-left: 5px;background: black; padding: 0;} .explanation{font-style: italic; color:#555;text-align:center;} a.hover{text-decoration:none;}</style><meta http-equiv="Content-Type" content="text/html; charset=utf-8"></head>';
    echo '<body>';
    echo '<div class="header">GENERATIVE API ART <span style="float: right"><a href="/">&#8635; REGÉNÉRER</a></span></div>';
    echo '<div class="explanation">Une représentation visuelle du JSON renvoyé par l\'api <a href="http://docs.art.rmngp.fr/">RMN-GP</a> pour une requête sur une œuvre aléatoire.</div>';
    echo "<h1><strong>" . $this->author . "</strong> - " . $this->title . "</h1>";
    
    echo '<div class="images"><img src="data:image/png;base64,'.base64_encode($image).'" width="' . $this->size . 'px" height="' . $this->size . 'px" />';

    echo '<div class="container"><img src="data:image/png;base64,'.base64_encode($real_image).'" width="' . $this->size . 'px" height="' . $this->size . 'px" /></div></div>';

    //echo '<div class="container"><img src="' . $this->real_image_url . '" class="" width="500px"/></div></div>';
    echo '<div class="well">Algorithme : <ul><li>Appel aléatoire à l\'API : <a href="' . $this->safe_url . '" target="_blank">' . $this->safe_url . '</a></li><li>Les caractères formant la réponse de l\'API sont groupés par trois</li><li>Ces trois valeurs forment une couleur RGB</li><li>Ces couleurs, misent bout à bout, forment une image de 500px &times 500px</li></ul></div>';
    echo "</body></html>";
  }

}
