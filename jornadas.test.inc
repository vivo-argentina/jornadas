<?php 

function lab_test($time){
 
        echo $time;
        $lapso=UN_MINUTO*15;
echo format_date($time)."<pre>";
 for($i=$time;$i<$time+UN_DIA;$i=$i+900){
        echo $i. "\n" ;
        process_clock_data($jid,$i);
        set_planta($i);
    }
        
    }