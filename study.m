% clear
% angle = [0,0,0,0,0,0,0,0,-192,-191,-190,-189,-188,-187,-186,-185,-184,-183,-182,-181,-180,-179,-178,-177,-176,-175,-174,-173,-172,-171,-170,-169,-168,-167,-166,-165,-164,-163,-162,-161,-160,-159,-158,-157,-156,-155,-154,-153,-152,-151,-150,-149,-148,-147,-146,-145,-144,-143,-142,-141,-140,-139,-138,-137,-136,-135,-134,-133,-132,-131,-130,-129,-128,-127,-126,-125,-124,-123,-122,-121,-120,-119,-118,-117,-116,-115,-114,-113,-112,-111,-110,-109,-108,-107,-106,-105,-104,-103,-102,-101,-100,-99,-98,-97,-96,-95,-94,-93,-92,-91,-90,-89,-88,-87,-86,-85,-84,-83,-82,-81,-80,-79,-78,-77,-76,-75,-74,-73,-72,-71,-70,-69,-68,-67,-66,-65,-64,-63,-62,-61,-60,-59,-58,-57,-56,-55,-54,-53,-52,-51,-50,-49,-48,-47,-46,-45,-44,-43,-42,-41,-40,-39,-38,-37,-36,-35,-34,-33,-32,-31,-30,-29,-28,-27,-26,-25,-24,-23,-22,-21,-20,-19,-18,-17,-16,-15,-14,-13,-12,-11,-10,-9,-8,-7,-6,-5,-4,-3,-2,-1,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,176,177,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0];
% t = [0,0,0,37364,520,31793,30002,28876,1552,1752,1966,2122,2287,2423,2572,2700,2858,2974,3096,3234,3337,3442,3580,3714,3827,3939,4046,4137,4238,4320,4472,4552,4675,4744,4828,4940,5007,5120,5198,5296,5370,5450,5510,5597,5689,5760,5831,5916,5992,6059,6143,6241,6299,6362,6458,6505,6576,6634,6723,6803,6872,6935,7002,7066,7133,7202,7276,7338,7418,7490,7548,7595,7670,7748,7795,7860,7902,7967,8020,8105,8165,8228,8288,8339,8417,8451,8505,8596,8634,8718,8754,8830,8874,8921,9004,9053,9082,9133,9213,9251,9322,9360,9436,9467,9521,9588,9623,9695,9757,9813,9873,9906,9971,10016,10076,10141,10194,10252,10305,10328,10390,10448,10506,10555,10615,10676,10734,10774,10823,10881,10934,10981,11019,11079,11142,11175,11251,11295,11344,11389,11445,11474,11532,11603,11636,11683,11743,11779,11837,11875,11942,11993,12051,12089,12156,12205,12247,12281,12357,12412,12468,12499,12557,12608,12675,12713,12760,12803,12867,12912,12958,13030,13039,13119,13168,13195,13262,13284,13349,13407,13436,13498,13549,13569,13629,13663,13750,13775,13841,13884,13939,13986,14049,14098,14133,14191,14216,14276,14327,14394,14437,14481,14546,14586,14628,14682,14742,14763,14836,14865,14929,14972,15023,15061,15130,15168,15228,15257,15315,15343,15418,15464,15498,15551,15607,15661,15685,15743,15808,15824,15881,15932,15961,16024,16075,16133,16184,16222,16260,16329,16350,16405,16472,16521,16550,16599,16650,16708,16760,16815,16864,16902,16962,17020,17078,17112,17185,17206,17272,17333,17379,17428,17471,17542,17598,17642,17694,17734,17787,17817,17914,17957,17986,18059,18084,18151,18204,18269,18327,18371,18418,18478,18512,18574,18634,18668,18739,18795,18838,18886,18953,19025,19074,19134,19183,19221,19288,19343,19424,19453,19533,19584,19631,19700,19754,19796,19867,19934,20012,20059,20115,20182,20240,20311,20360,20407,20489,20540,20603,20694,20752,20799,20873,20915,20997,21064,21111,21191,21256,21315,21381,21466,21517,21604,21680,21758,21824,21878,21960,22032,22099,22192,22284,22353,22424,22502,22591,22656,22734,22814,22908,22992,23055,23153,23231,23327,23429,23505,23590,23675,23764,23848,23978,24071,24161,24283,24379,24470,24580,24663,24800,24918,25048,25137,25268,25433,25538,25712,7100,570,902,1569,1907,2246,2574,2930,3262,3547,3855,4116,4546,4954,5230,5535,50724,30116,23933,9810,6394,4838];
% angle = angle(9:400 - 22);
% t = t(9:400 - 22);
% 
% t2 = 30:30:30000;
% angle2(1:length(t2)) = angle(length(angle));
% 
% current_t2_index = 1;
% 
% for i = 1:length(t)
%     for j = current_t2_index:length(t2)
%         if t2(j) < t(i)
%             angle2(j) = angle(i);
%         else
%             current_t2_index = j;
%             break;
%         end
%     end
% end
% figure(1);
% plot(t2,angle2);
% 
% fft_ = fft(angle2);
% figure(2);
% plot(abs(fft_));

% angle2(length(angle)) = 0;
% t2(length(t)) = 0;
% ff = fft(t);
% plot(1:length(ff),abs(ff));
% %ff(1:length(ff)/2 - 120) = 0;
% ff(100:length(ff)) = 0;
% iff = ifft(ff);
% figure(2)
% plot(iff,1:length(iff))
% hold on
% plot(t,angle)

clear
t = 0.001:0.001:1.0;
y = sin(2*pi/1*t);
y2 = 0.05*sin(2*pi/0.003*t);
y3 = y + y2
plot(t,y3)