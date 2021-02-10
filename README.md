# Reliable-Data-Transport-Protocol-Simulation-with-C
## 1. PROJENİN AMACI
Bu projede amaç güvenilir bir veri aktarma protokolü uygulamak için Transport Level Code gönderen ve alan bir yapı oluşturmak. İki şekilde yapılabilir:
•	Alternating-Bit-Protocol sürümü
•	Go-Back-N sürümü

## 2.	PROJENİN İÇERİĞİ

** 2.1	Mesaj, Paket ve Event Yapıları **
Message(msg) yapısı; 20 elemandan oluşan bir char dizisidir. Program boyunca gönderilen mesajlar bu türden olacaktır.

Packet(pkt) yapısı; sequence number, ack number, checksum ve payload’dan oluşur. Program boyunca gönderilen paketler bu türden olacaktır.
•	Seqnum: Paketin sıra numarasıdır.
•	Acknum: Paketin ACK numarasıdır.
•	Checksum: Paketin bit kontrollerini yapar.
•	Payload: Paketin boyutunu belirtir.

Event yapısı; evtime, evtype, eventity, paket işaretçisi, önceki ve sonraki olayları içerir.
•	Evtime: Olayın gerçekleşme zamanını tutar.
•	Evtype: Olayın türünü tutar.
•	Eventity: Olayın hangi varlık üzerinde gerçekleştiğini tutar.
•	Pktptr: Paket işaretçisidir.
•	Prev: Önceki olayı tutar.
•	Next: Sonraki olayı tutar.

# 2.2	Değişkenler ve Fonksiyonlar
A_output: Üst katmanlardan bir mesaj alır. Sadece iki durumda olabilir;
1)	A_output bir cevap gönderiyor.
2)	A_output bir cevap bekliyor.
A_STATE: A_output’un durumunu belirtir. Bir cevap gönderiliyorsa, paketi oluşturur ve daha sonra bu paketi, yeniden iletme için kullanılan “prev_packet” değişkenine kaydeder. 
A_input: Aldığı paketin beklediğimiz ACK’ya sahip olup olmadığını, zamanlayıcı zaman aşımına uğramış mı kontrol eder. Aynı zamanda paketin sağlama toplamını da kontrol eder. 
A_timerinterrupt: A'nın zamanlayıcı süresi dolduğunda ve zamanlayıcıyı yeniden başlattığında prev_packet'i yani bir önceki paketi gönderir.
A_init: A_STATE, ACK ve SEQ değerini 0 olarak ayarlayan fonksiyondur.

B_output: Üst katmanlardan bir mesaj alır. Sadece iki durumda olabilir;
1)	B_output bir cevap gönderiyor.
2)	B_output bir cevap bekliyor.
B_STATE: B_output’un durumunu belirtir. Bir cevap gönderiliyorsa, paketi oluşturur ve daha sonra bu paketi, yeniden iletme için kullanılan “prev_packet” değişkenine kaydeder. 
B_input: Aldığı paketin beklenen ACK’ya sahip olup olmadığını ve zamanlayıcının zaman aşımına uğramasını kontrol eder. Ayrıca paketin sağlama toplamını da kontrol eder.
B_timerinterrupt: B’nin zamanlayıcı süresi dolduğunda ve zamanlayıcıyı yeniden başlattığında prev_packet'i yani bir önceki paketi gönderir.
B_init: Sadece B_STATE ayarını 0 yapar.

starttimer(calling_entity, increment): Burada calling_entity, 0 (A tarafı zamanlayıcısını başlatmak için) veya 1 (B tarafı zamanlayıcısını başlatmak için) değerini alırken, increment zamanlayıcı kesintisinden önce geçen süreyi belirten float bir değerdir.
stoptimer (calling_entity): Burada calling_entity, 0 (A tarafı zamanlayıcıyı durdurmak için) veya 1 (B tarafı zamanlayıcıyı durdurmak için) değerini alır.
tolayer3(calling_entity, packet): Burada calling_entity, 0 (A tarafı gönderme için) veya 1 (B tarafı gönderme için) değerini alırken, packet pkt türünde bir yapıdır. Bu fonksiyonu çağırmak, paketin üçüncü katmana gönderildiği anlamına gelir.
tolayer5 (calling_entity, message): Burada calling_entity 0 (katman 5'e A tarafı teslimatı için) veya 1 (katman 5'e B tarafı teslimatı için) değerini alırken, message msg tipinde bir yapıdır. Bu fonksiyonu çağırmak, paketin beşinci katmana gönderildiği anlamına gelir.
check_checksum: Giden paketlerimizde sağlama toplamı oluşturmak ve kontrol etmek için yardımcı fonksiyondur. Check_checksum bu paketi kabul edersek veya reddedersek bir boolean değer döndürür. 
generate_checksum: Giden paketlerimizde sağlama toplamı oluşturmak ve kontrol etmek için yardımcı fonksiyondur. Toplam dışındaki tüm alanları (taşma biti gibi) pakete ekleyerek sağlama toplamını hesaplar.
flip_number: Bu fonksiyon, SEQ ve ACK bitini değiştirmeye yardımcı olur. İnteger bir değer alır ve bu değer 0 ise 1 döndürür, 1 ise 0 döndürür.

## 2.3	Girdiler
Number of Messages to Simulate: İletilerin tümünün doğru iletilip iletilmediğine bakılmaksızın, girilen sayıda ileti katman 5'e iletilir. Bu değeri 1 olarak ayarlarsanız, mesaj diğer tarafa iletilmeden program sona erecektir. Bu nedenle, bu değer her zaman 1'den büyük olmalıdır.
Packet Loss Probability: Bir paket kaybı olasılığı belirtmeniz istenir. 0.1 değeri, ortalama olarak on paketten birinin kaybolduğu anlamına gelir. 0.0 ise paket kaybı yok demektir.
Packet Corruption Probability: Paket bozulma olasılığı belirtmeniz istenir. 0.2 değeri, ortalama olarak beş paketten birinin bozuk olduğu anlamına gelir. Payload, sequence, ack veya checksum kısımları da bozulabilir. Bu durumda checksum kısmının data, sequence, ve ack alanlarını içermesi gerekir. 0.0 ise paket bozulması yok demektir.
Average Time Between Messages from Sender’s Layer5: Bu değeri sıfır olmayan, pozitif bir değere ayarlayabilirsiniz. Seçtiğiniz değer ne kadar küçük olursa, paketler o kadar hızlı göndericiye ulaşacaktır.
Random Number Generator: Programın doğru çalışması için random bir değer üretir.
Trace: Bu değer paketleri izlemeye yardımcı olur. 1 veya 2 değerinde bir izleme değeri ayarlamak, neler olup bittiğiyle alakalı yararlı bilgiler yazdıracaktır. 0 girmek bu özelliği kapatır. 3 girdiğinizde her türlü mesajı görüntüler.

