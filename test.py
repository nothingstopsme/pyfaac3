import pyfaac3
import os
import time
import threading

def test_configuration():
  #creating an encoder
  with pyfaac3.Encoder(sample_rate=8000, number_of_channels=2) as encoder:
    assert encoder.sample_rate == 8000
    assert encoder.number_of_channels == 2

    # getting a copy of the current configuration
    config = encoder.get_configuration()

    config.bit_rate = 32000
    encoder.set_configuration(config)
    assert encoder.get_configuration().bit_rate == 32000


    # testing if configurations can be updated successfully; for the purpose of testing, settings are modified and checked individually, which is not necessary in the normal usage (multiple setting changes can be applied at the same time).
    for aac_object_type in (pyfaac3.LOW, pyfaac3.LTP, pyfaac3.MAIN):
      config.aac_object_type = aac_object_type
      encoder.set_configuration(config)
      assert encoder.get_configuration().aac_object_type == aac_object_type



    for mpeg_version in (pyfaac3.MPEG2, pyfaac3.MPEG4):
      config.mpeg_version = mpeg_version
      encoder.set_configuration(config)
      assert encoder.get_configuration().mpeg_version == mpeg_version


    for use_lfe in (0, 1):
      config.use_lfe = use_lfe
      encoder.set_configuration(config)
      assert encoder.get_configuration().use_lfe == use_lfe

    for use_tns in (0, 1):
      config.use_tns = use_tns
      encoder.set_configuration(config)
      assert encoder.get_configuration().use_tns == use_tns



    config.bandwidth = 2000
    encoder.set_configuration(config)
    assert encoder.get_configuration().bandwidth == 2000

    config.quantqual = 50
    encoder.set_configuration(config)
    assert encoder.get_configuration().quantqual == 50

    for output_format in (pyfaac3.RAW, pyfaac3.ADTS):
      config.output_format = output_format
      encoder.set_configuration(config)
      assert encoder.get_configuration().output_format == output_format


    for input_format in (pyfaac3.FAAC_INPUT_16BIT, pyfaac3.FAAC_INPUT_32BIT, pyfaac3.FAAC_INPUT_FLOAT):
      config.input_format = input_format
      encoder.set_configuration(config)
      assert encoder.get_configuration().input_format == input_format


    for shortctl in (pyfaac3.SHORTCTL_NORMAL, pyfaac3.SHORTCTL_NOSHORT, pyfaac3.SHORTCTL_NOLONG):
      config.shortctl = shortctl
      encoder.set_configuration(config)
      assert encoder.get_configuration().shortctl == shortctl

    for ch_index in range(64):
      config.channel_map[ch_index] = 64 - ch_index - 1
   
    encoder.set_configuration(config)
    updated_channel_map = encoder.get_configuration().channel_map
    for ch_index in range(64):
      assert updated_channel_map[ch_index] == 64 - ch_index - 1 


def test_encoding(encoder, pcm_path, output_name):

  frame_size = encoder.input_sample_size * encoder.number_of_samples_per_frame
 
  with open(pcm_path, 'rb') as fin:
    output_path = os.path.join(os.path.dirname(pcm_path), output_name)
    with open(output_path, 'wb') as fout:
      while True:
        pcm = fin.read(frame_size)
        if pcm:
          aac = encoder.encode(pcm)
          # Note: no aac data (empty buffers) will be returned at the first few calls to encode(), as the encoding process is waiting for the internal buffer to get filled up;
          # so there is actually a delay in aac frames coming out of the encoder, which should be taken into account in the case of synchronisation
          fout.write(aac)
        else:
          break
          
      # reaching the end of the input file, but there are a few frames still cached in the encoder's internal buffer, so repeatingly calling flush() to return each cached audio frame, until the returned buffer is empty (indicating no frame is cached in the encoder)
      while True:
        aac = encoder.flush()
        if aac:
          fout.write(aac)
        else:
          break


def main():
  test_configuration()

  #using pyfaac3.Encoder in 'with statement'
  with pyfaac3.Encoder(sample_rate=16000, number_of_channels=1) as encoder:
    config = encoder.get_configuration()
    config.bit_rate = 32000
    config.mpeg_version = pyfaac3.MPEG2
    config.input_format = pyfaac3.FAAC_INPUT_16BIT
    config.output_format = pyfaac3.ADTS
    config.aac_object_type = pyfaac3.MAIN
    encoder.set_configuration(config)

    test_encoding(encoder, 'test.pcm', 'test_0.aac')


  #using pyfaac3.Encoder in a multithreading context without 'with' (therefore users need to close encoders by themselves; 
  #otherwise the corresponding resource will not be released until their encoders get garbage-collected)
  encoders = [pyfaac3.Encoder(sample_rate=16000, number_of_channels=1) for _ in range(10)]
  bit_rates = [16000, 32000, 64000]
  object_types = [pyfaac3.MAIN, pyfaac3.LOW, pyfaac3.LTP]
  for encoder_index in range(len(encoders)):
    config = encoders[encoder_index].get_configuration()
    config.bit_rate = bit_rates[encoder_index%len(bit_rates)]
    config.mpeg_version = pyfaac3.MPEG4
    config.input_format = pyfaac3.FAAC_INPUT_16BIT
    config.output_format = pyfaac3.ADTS
    config.aac_object_type = object_types[encoder_index%len(object_types)]
    encoders[encoder_index].set_configuration(config)


  threads = [threading.Thread(target=test_encoding, args=(encoders[t], 'test.pcm', 'test_{}.aac'.format(t+1))) for t in range(10)]
  for thread in threads:
    thread.start()

  for index in range(len(threads)):
    threads[index].join()
    encoders[index].close()




if __name__ == '__main__':
  main()

